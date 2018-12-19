#include <Arduino.h>
#include "Thesis.h"

/*
MOSI connects pin 4 on Master to pin 3 on Slave (leaving pin 2
free in case interrupts are needed in the future. Likewise, MISO
connects pin 4 on Slave to pin 3 on Master.
*/
#define MOSI_HIGH PORTD |= 0x10
#define MOSI_LOW PORTD &= 0xef
#define MISO PIND & 0x08

static uint8_t inputByte;
static uint8_t inputByteArray[255];

static uint8_t errorFlag;
static uint32_t ticksElapsed;

static uint8_t failOuterLoop;
static uint8_t failInnerLoop;
static uint8_t failWrongValue;
static uint8_t failWrongCheck;

static inline void cmdWrite(uint8_t cmd){
  ENABLE_WRITE;
  WRITE_BUS(cmd);
  MOSI_HIGH;
  while(~MISO);//Wait for acknowledgement. TODO add a timeout
  ENABLE_READ;
  MOSI_LOW;
  while(MISO);
}

static inline void cmdRead1(){
  cmdWrite((uint8_t) CMD_READ_1);
  MOSI_HIGH;
  while(~MISO);
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
  while(~MISO);
  MOSI_LOW;
  while(MISO);//wait for low
  
  cli();
  
  ticksElapsed = 0;
  TCNT2 = 0;

  while(outerCounter < N){
    while(innerCounter < 255){
      while(~MISO);//wait for high
      //The "real" line of code is:
      //inputByteArray[innerCounter] = 0x0f & PINB;
      //which will be handled at the bottom of the outer loop
      //For the sake of testing, we just check one byte at a time on the fly.
      inputByte = 0x0f & PINB;
      if(inputByte != (0x0f & innerCounter)) {
        errorFlag = 1;
        sei();
        return;
      }

      innerCounter++;
            
      while(MISO);//wait for low
      ticksElapsed += TCNT2;
      TCNT2 = 0;
    }
    
    //Do something with the array here
    innerCounter = 0;
    outerCounter++;
  }
  sei();

}

static inline void cmdReset(){
  MOSI_HIGH;
  while(~MISO);
  MOSI_LOW;
  cmdWrite((uint8_t) CMD_RESET);
}

static inline void initialize(){
  errorFlag = 0;
  TCCR2B |= 0b00000101; //Prescaler of 1024
  DDRD |= 0b00010000;
  DDRD &= 0b11110111;  
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Master beginning");
  delay(1000);
  initialize();
  //change the prescaler here
  //delay(1000);
}

void loop() {
  // put your main code here, to run repeatedly:
  int blocks = 15;
  cmdReadN(blocks);

  if (errorFlag == 0){
    Serial.println("Transmission was successful."); 
  } else {
    Serial.println("Transmission failed.");
  }

  //system clock period is 62.5 nS
  //Prescaler is 1024
  //Time in seconds is 6.25e-8 * 1024 * ticksElapsed

  double transmissionTime = 6.25e-8 * 1024 * ticksElapsed;
  Serial.print("Total transmission time = ");
  Serial.print(transmissionTime);
  Serial.println(" seconds.");

  //One byte requires two transmissions
  //Total bytes transmitted is thus blocks * 255 / 2;
  //Throughput = bytes transmitted / transmissionTime

  Serial.print("Throughput = ");
  Serial.print(blocks * 127.5 / transmissionTime);
  Serial.println(" bytes per second");
  
  while(1);
}
