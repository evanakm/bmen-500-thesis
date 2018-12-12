#include <Arduino.h>
#include "ThesisMaster.h"

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  initialize();
  //change the prescaler here
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
  Serial.println(blocks * 127.5 / transmissionTime);
}
