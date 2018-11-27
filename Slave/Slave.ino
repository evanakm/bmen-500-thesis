//Slave
#include <Arduino.h>
#include <avr/power.h>
#include <avr/sleep.h>

const byte interruptPinRx = 3;
const byte interruptPinTx = 4;

volatile byte interruptStateRx = LOW;
volatile byte interruptStateTx = LOW;
volatile uint8_t counter = 0;

void setup() {
  Serial.begin(115200);
  DDRB |= 0x3f; //set D13-D8 as outputs
  pinMode(interruptPinRx, INPUT);
  pinMode(interruptPinTx, OUTPUT);
  PORTD &= 0b11101111; //Set port 4 low
  set_sleep_mode(SLEEP_MODE_STANDBY);
  attachInterrupt(digitalPinToInterrupt(interruptPinRx), writeByteToBus, RISING);
  InitializeSlave();
  delay(5000);
}

void writeByteToBus() {
    PORTB = counter;
    PORTD |= 0b00010000; //Set pin 4 high, triggering interrupt on master
    counter = (counter + 1) & 0xff; //ANDing is not necessary, but it's a guard against the type changing to a uint16 in the future.
    //counter = (counter + 1) % 50;
    //counter = 42;
    PORTD &= 0b11101111; //Set pin 4 low.
}

void loop() {

  sleep_mode(); //Waiting to be triggered by master.
  
  // TODO: TEST B: Round Trip
}

bool InitializeSlave () {
  PORTB &= 0x3f; //Set all bits to zero.
  Serial.println("Slave ready");
}

bool endTransmit() {

  return false;
}
