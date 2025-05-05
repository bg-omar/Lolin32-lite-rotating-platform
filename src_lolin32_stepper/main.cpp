#include <Arduino.h>
#include "stepper_main.h"


void setup() {
	Serial.begin(9600);
	Serial.println("ESP32 Water bottom plate Rotating Version: 0.0.1 ");
	stepper_main::stepperSetup();
}

void loop() {
	stepper_main::stepperLoop();
}
