// Originally written November 2018
// by Evan Meikleham

#ifndef _THESIS_SLAVE_H_
#define _THESIS_SLAVE_H_

#include "Thesis.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
MOSI connects pin 4 on Master to pin 3 on Slave (leaving pin 2
free in case interrupts are needed in the future. Likewise, MISO
connects pin 4 on Slave to pin 3 on Master.
*/
#define MISO_HIGH PORTD |= 0x10
#define MISO_LOW PORTD &= 0xef
#define MOSI PINB & 0x08

/*
Pins 8 to 13 (on Master and Slave) are going to be the bus.
For now we're only using the 4 LSB, since a byte requires two
read cycles anyway.
*/
#define ENABLE_WRITE DDRB |= 0x3f
#define ENABLE_READ DDRB &= 0xc0

#define WRITE_BUS(x) PORTB = (x)
#define READ_BUS inputByte = PINB

enum state { 	STATE_READY = 0b00000000,
				STATE_READING = 0b00000001,
				STATE_WRITE_1 = 0b00000100,
				STATE_WRITE_N_STEP_1 = 0b00000101,
				STATE_WRITE_N_STEP_2 = 0b00000110,
				STATE_INTERRUPTED = 0b00001000};

static uint8_t readByte;
static uint8_t numberToWrite; 
static uint8_t dummy;

static enum state currentState;

static inline void reset(){
	//Reset any other global variables
	currentState = STATE_READY;
	dummy = 0; //For testing. Will be removed.
}
				
/*
writeToBus assumes uint fits into 6 bits.
This function is meant to be called immediately after MOSI rising edge,
i.e. it assumes a precondition of MOSI being high.
*/
static inline void writeToBus(uint8_t dataByte){
	ENABLE_WRITE;
	WRITE_BUS(dataByte);
	MISO_HIGH;
	while(MOSI);//wait for Master to acknowledge
	ENABLE_READ;
	MISO_LOW;
}

static inline void writeNBytes(uint8_t N){

	static uint8_t outerCounter = 0;
	static uint8_t innerCounter = 0;
		
	ENABLE_WRITE;
	WRITE_BUS(0); //Throwaway byte
	MISO_HIGH;
	while(MOSI);
	MISO_LOW;
	
	cli();
	
	while(outerCounter < N){
		while(innerCounter < 255){
			if(currentState == STATE_INTERRUPTED) {
				//MISO is low at this point, so does not need to be reset
				currentState = STATE_READY;
				ENABLE_READ;
				sei();
				return;
			}
			
			WRITE_BUS(innerCounter & 0x0f);
			MISO_HIGH;
			//Insert some NOPs here
			MISO_LOW;
			//Insert some NOPs here
			innerCounter++;
		}
		innerCounter = 0;
		outerCounter++;
	}
	
	ENABLE_READ;
	sei();
}

static inline void mapCommandToState(uint8_t cmd){
	switch((enum state) cmd){
		case CMD_RESET:
			currentState = STATE_READY;
			reset();
			break;
		case CMD_WRITE:
			currentState = STATE_READING;
			break;
		case CMD_READ_1:
			currentState = STATE_WRITE_1;
			break;
		case CMD_READ_N:
			currentState = STATE_WRITE_N_STEP_1;
			break;
	}
	
}

/*main while loop will just call this function repeatedly.*/
static inline void waitForRisingEdge(){
	switch (currentState){
		case STATE_READY:
			while(!MOSI);
			readByte = 0x3f & PINB;
			mapCommandToState(readByte);
			break;
		case STATE_WRITE_1:
			while(!MOSI);
			readByte = 0x3f & PINB;
			//Either the data would be primed to write, or else it would be acquired here.
			//For the sake of testing, just send a dummy value.
			writeToBus(dummy++ & 0x0f);
			break;
		case STATE_WRITE_N_STEP_1:
			while(!MOSI);
			numberToWrite = 0x3f & PINB; //readByte could be used for this, but keeping the code readable.
			break;
		case STATE_WRITE_N_STEP_2:
			while(!MOSI);
			writeNBytes(numberToWrite);
			break;
		default:
		    /*
			In theory it shouldn't reach this point in STATE_INTERRUPTED.
			We haven't implemented anything in STATE_READING yet, so it behaves like STATE_READY
			*/
			while(!MOSI);
			readByte = 0x3f & PINB;
			mapCommandToState(readByte);
			break;		
	}
		
}

/*
This will be called from the writing state, that is, the Slave has control of the bus.
*/
static inline void onInterrupt(){
	currentState = STATE_INTERRUPTED;
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif //include guard
