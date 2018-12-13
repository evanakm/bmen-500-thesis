#include <Arduino.h>
#include "Thesis.h"

/*
MOSI connects pin 4 on Master to pin 3 on Slave (leaving pin 2
free in case interrupts are needed in the future. Likewise, MISO
connects pin 4 on Slave to pin 3 on Master.
*/
#define MISO_HIGH PORTD |= 0x10
#define MISO_LOW PORTD &= 0xef
#define MOSI PIND & 0x08

#define DISABLE_TIMER_INTERRUPT TIMSK0 &= ~(1 << TOIE0)
#define ENABLE_TIMER_INTERRUPT TIMSK0 |= (1 << OCIE0A)

enum state {   STATE_READY = 0b00000000,
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
  MISO_LOW;
}

static inline void echoMaster(){
  NOP;
  NOP;
  NOP;
  NOP;
  MISO_HIGH;
  while(MOSI);
  NOP;
  NOP;
  NOP;
  NOP;
  MISO_LOW;
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

  //attachInterrupt(digitalPinToInterrupt(3), onInterrupt, RISING);
  DISABLE_TIMER_INTERRUPT;

  while(outerCounter < N){
    //Serial.print("outerCounter = ");
    //Serial.println(outerCounter);
    while(innerCounter < 255){
      //Serial.print("innerCounter = ");
      //Serial.println(innerCounter);
      if(currentState == STATE_INTERRUPTED) {
        Serial.println("Maybe it's interrupted...");      
        //MISO is low at this point, so does not need to be reset
        currentState = STATE_READY;
        ENABLE_READ;
        ENABLE_TIMER_INTERRUPT;
        //detachInterrupt(digitalPinToInterrupt(3));
        return;
      }

      WRITE_BUS(innerCounter & 0x0f);
      MISO_HIGH;
      NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;
      MISO_LOW;
      NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;
      innerCounter++;
    }
    innerCounter = 0;
    outerCounter++;
  }
  
  ENABLE_READ;
  ENABLE_TIMER_INTERRUPT;
  //detachInterrupt(digitalPinToInterrupt(3));
  
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
    default:
      //Serial.println("Unrecognized command");
      currentState = STATE_READY;
      break;    
  }
  
}

/*main while loop will just call this function repeatedly.*/
static inline void waitForRisingEdge(){

/*  if((uint8_t) MOSI == 0) Serial.println("Does it find the 'if'?");
  else {
    Serial.println("Does it find the 'else'?");
    Serial.print("MOSI = ");
    Serial.println(MOSI, HEX);
    Serial.print("MOSI == 0 = ");
    Serial.println(MOSI == 0);
    Serial.print("PIND = ");
    Serial.println(PIND, HEX);
  }
*/
  //while(~MOSI) Serial.println("Does it get here?");
  
  switch (currentState){
    case STATE_READY:
      while(~MOSI);
      readByte = 0x3f & PINB;
      Serial.print("readByte = ");
      Serial.println(readByte);      
      mapCommandToState(readByte);
      echoMaster();
      break;
    case STATE_WRITE_1:
      while(~MOSI);
      readByte = 0x3f & PINB;
      Serial.print("readByte = ");
      Serial.println(readByte);      
      //Either the data would be primed to write, or else it would be acquired here.
      //For the sake of testing, just send a dummy value.
      writeToBus(dummy++ & 0x0f);
      break;
    case STATE_WRITE_N_STEP_1:
      while(~MOSI);
      numberToWrite = 0x3f & PINB; //readByte could be used for this, but keeping the code readable.
      Serial.print("numberToWrite = ");
      Serial.println(numberToWrite);
      currentState = STATE_WRITE_N_STEP_2;
      echoMaster();
      break;
    case STATE_WRITE_N_STEP_2:
      while(~MOSI);
      Serial.println("Now writing");
      writeNBytes(numberToWrite);
      break;
    default:
        /*
      In theory it shouldn't reach this point in STATE_INTERRUPTED.
      We haven't implemented anything in STATE_READING yet, so it behaves like STATE_READY
      */
      while(~MOSI);
      readByte = 0x3f & PINB;
      Serial.print("readByte = ");
      Serial.println(readByte);      
      mapCommandToState(readByte);
      echoMaster();
      break;    
  }
    
}

/*
This will be called from the writing state, that is, the Slave has control of the bus.
*/
static inline void onInterrupt(){
  Serial.println("Interrupt triggered");
  currentState = STATE_INTERRUPTED;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Starting the Slave");
  DDRD |= 0b00010000;
  DDRD &= 0b11110111;
  reset();
}

void loop() {
  //MISO_HIGH;
  waitForRisingEdge();
  // put your main code here, to run repeatedly:
}
