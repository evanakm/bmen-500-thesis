#include <Arduino.h>
#include <Thesis.h>

const int pulseWidth_us = 500;
Slave slave(SLAVE_PIN_TO_MASTER, SLAVE_PIN_FROM_MASTER, pulseWidth_us);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  attachInterrupt(digitalPinToInterrupt(SLAVE_PIN_FROM_MASTER, onRisingEdgeOfMasterToSlave, RISING);
}

void loop() {
  // put your main code here, to run repeatedly:

}
