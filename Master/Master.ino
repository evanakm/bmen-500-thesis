//Master
#include <Arduino.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>

const byte interruptPinRx = 3;
const byte interruptPinTx = 4;
const int NUMBER_OF_SAMPLES = 10000;

volatile byte interruptStateRx = LOW;
volatile byte interruptStateTx = LOW;
volatile uint16_t counter = 0;

uint8_t storage[64];

void setup() {
  Serial.begin(115200);
  DDRB &= 0xc0; //set D13-D8 as inputs
  pinMode(interruptPinRx, INPUT);
  pinMode(interruptPinTx, OUTPUT);
  PORTD &= 0b11101111; //Set port 4 low
  set_sleep_mode(SLEEP_MODE_STANDBY);
  //sleep_enable();
  attachInterrupt(digitalPinToInterrupt(interruptPinRx), readByteFromBus, RISING);
}

void readByteFromBus(){
  storage[counter & 0x3f] = PINB;
  counter++;
  PORTD &= 0b11101111; //sets interruptTx low to get ready for next
}

void loop() {

  InitializeMaster();
  delay(5000);
  float timestart = millis();

  do{
    //Serial.print("counter = ");
    //Serial.println(counter);
    //delayMicroseconds(500);
    PORTD |= 0b00010000; //Set port 4 high, triggering interrupt on slave
    sleep_mode(); //Wait for response, will be interrupted. counter++ happens after the interrupt
    //counter++;
  }while(counter < NUMBER_OF_SAMPLES);

  float timeend = millis();
  float timeKBRead = timeend - timestart;
  Serial.print("Time taken in milliseconds for 10000 bytes of data to be read is: ");
  Serial.println(timeKBRead);
  Serial.print("Readings started: ");
  Serial.println(timestart);
  Serial.print("Readings finished: ");
  Serial.println(timeend);
  delay(5000);

  //data integrity calcs?
  for(int i = 0; i < 64; i++){
    Serial.println(storage[i]);
  }

  detachInterrupt(digitalPinToInterrupt(interruptPinRx));
  Serial.println("Entering sleep mode.");
  delay(5000);
  sleep_mode();
}

void InitializeMaster(void){
   Serial.println("Master starting");
}
