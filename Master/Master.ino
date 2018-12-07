#include <Arduino.h>
#include <Thesis.h>

const int pulseWidth_us = 500;
Master master(MASTER_PIN_TO_SLAVE, MASTER_PIN_FROM_SLAVE, pulseWidth_us);
const int NUMBER_OF_SAMPLES = 10000;
uint8_t storage[64];

uint16_t counter = 0;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(5000);
  float timestart = millis();

  do{
    master.enterWriteState();
    master.requestAndReadOneByte();
    if(master.getState() == MASTER_STATE_FAILED) continue;
    storage[counter % 64] = 0x3f & master.getReadByte();
    counter++;
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

}
