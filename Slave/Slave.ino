#include <Arduino.h>
#include <ThesisSlave.h>

void onRisingEdgeOfMOSI(){
  onInterrupt();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  attachInterrupt(digitalPinToInterrupt(3), onRisingEdgeOfMOSI, RISING);
  reset();
}

void loop() {
  waitForRisingEdge();
  // put your main code here, to run repeatedly:
}
