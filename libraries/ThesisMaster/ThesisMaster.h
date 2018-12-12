// Originally written November 2018
// by Evan Meikleham

#ifndef _THESIS_MASTER_H_
#define _THESIS_MASTER_H_

#include "Thesis.h"

#ifdef __cplusplus
extern "C" {
#endif

static uint8_t inputByte;
static uint8_t inputByteArray[255];

static uint8_t errorFlag;
static uint16_t ticksElapsed;

/*
MOSI connects pin 4 on Master to pin 3 on Slave (leaving pin 2
free in case interrupts are needed in the future. Likewise, MISO
connects pin 4 on Slave to pin 3 on Master.
*/
#define MOSI_HIGH PORTD |= 0x10
#define MOSI_LOW PORTD &= 0xef
#define MISO PINB & 0x08

/*
Pins 8 to 13 (on Master and Slave) are going to be the bus.
For now we're only using the 4 LSB, since a byte requires two
read cycles anyway.
*/
#define ENABLE_WRITE DDRB |= 0x3f
#define ENABLE_READ DDRB &= 0xc0

#define WRITE_BUS(x) PORTB = (x)
#define READ_BUS inputByte = PINB

static inline void cmdWrite(uint8_t cmd){
	ENABLE_WRITE;
	WRITE_BUS(cmd);
	MOSI_HIGH;
	while(!MISO);//Wait for acknowledgement. TODO add a timeout
	ENABLE_READ;
	MOSI_LOW;
	while(MISO);
}

static inline void cmdRead1(){
	cmdWrite((uint8_t) CMD_READ_1);
	MOSI_HIGH;
	while(!MISO);
	READ_BUS;
	MOSI_LOW;
	while(MISO);
}

static inline void cmdReadN(uint8_t N){
	
	static uint8_t outerCounter = 0;
	static uint8_t innerCounter = 0;
	
	cmdWrite((uint8_t) CMD_READ_N);
	cmdWrite(N);
	MOSI_HIGH;
	while(!MISO);
	MOSI_LOW;
	while(MISO);//wait for low
	
	cli();
	
	ticksElapsed = 0;
	TCNT0 = 0;
	
	while(outerCounter < N){
		while(innerCounter < 255){
			while(!MISO);//wait for high
			//The "real" line of code is:
			//inputByteArray[innerCounter] = 0x0f & PINB;
			//which will be handled at the bottom of the outer loop
			//For the sake of testing, we just check one byte at a time on the fly.
			inputByte = 0x0f & PINB;
			if(inputByte != 0x0f & innerCounter) errorFlag = 1;

			innerCounter++;
						
			while(MISO);//wait for low
			ticksElapsed += TCNT0;
			TCNT0 = 0;
		}
		
		//Do something with the array here
		innerCounter = 0;
		outerCounter++;
	}
	sei();

}

static inline void cmdReset(){
	MOSI_HIGH;
	while(!MISO);
	MOSI_LOW;
	cmdWrite((uint8_t) CMD_RESET);
}

static inline void initialize(){
	errorFlag = 0;
	TCCR0B |= 0b00000101; //Prescaler of 1024
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif //include guard
