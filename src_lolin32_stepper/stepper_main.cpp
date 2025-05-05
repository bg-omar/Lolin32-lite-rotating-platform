//
// Created by mr on 7/23/2024.
//
#include "Arduino.h"
#include "stepper_main.h"

// Example sketch to control a stepper motor with A4988 stepper motor driver
// and Arduino without a library.
// More info: https://www.makerguides.com

// Define stepper motor connections and steps per revolution:
#define dirPin 5
#define stepPin 6
#define stepsPerRevolution 200

#define ANALOG_IN 7

int dir = 0;
int analogInt = 0;


void stepper_main::stepperSetup() {
	// Declare pins as output:
	pinMode(stepPin, OUTPUT);
	pinMode(dirPin, OUTPUT);
}

void stepper_main::stepperLoop() {
	int analog_in = analogRead(ANALOG_IN);
	Serial.print("analog_in ");Serial.println(analog_in);
	if (analog_in) {
		if (analog_in < 657) {
			digitalWrite(dirPin, HIGH);
			analogInt = map(analog_in, 0, 657, 3, 10);
		} else {
			digitalWrite(dirPin, LOW);
			analogInt = map(analog_in, 657, 818, 10, 3);
		}
	}
	Serial.print("analogInt ");Serial.println(analogInt);

	for (int i = 0; i < stepsPerRevolution; i++) {
		// These four lines result in 1 step:
		digitalWrite(stepPin, HIGH);
		delay(analogInt);
		digitalWrite(stepPin, LOW);
		delay(analogInt);
	}
}