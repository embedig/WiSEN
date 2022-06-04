#include "WiSEN.h"

// Variables will change:
int ledState = OFF;             // ledState used to set the LED

unsigned long previousMillis = 0;        // will store last time LED was updated

// constants won't change:
const long interval = 1000;           // interval at which to blink (milliseconds)

void setup() {
  WiSEN.LED(ledState);
}

void loop() {
  // check to see if it's time to blink the LED; that is, if the difference
  // between the current time and last time you blinked the LED is bigger than
  // the interval at which you want to blink the LED.
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // set opposite the LED with the ledState of the variable:
    ledState = ~ledState;
    
    WiSEN.LED(ledState);
  }
}
