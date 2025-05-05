#include "main.h"
#include "Arduino.h"
// Include the Arduino Stepper Library
#include <Stepper.h>

// Steps per revolution (200 for NEMA 17)
const int stepsPerRevolution = 200;

// initialize the stepper library on pins 8 through 11:
Stepper myStepper(stepsPerRevolution, 14, 12, 13, 15);

int stepCount = 0;  // number of steps the motor has taken

#define STEP_PIN 14  // D5 on Wemos Lolin D1 Mini (GPIO 14)
#define DIR_PIN  12  // D6 (GPIO 12)
#define ENABLE_PIN 13 // D7 (GPIO 13, active LOW)
#define MS1 4  // GPIO 4 (D2)
#define MS2 5  // GPIO 5 (D1)
#define MS3 0  // GPIO 0 (D3)


void setup() {
	pinMode(STEP_PIN, OUTPUT);
	pinMode(DIR_PIN, OUTPUT);
	pinMode(ENABLE_PIN, OUTPUT);
	pinMode(MS1, OUTPUT);
	pinMode(MS2, OUTPUT);
	pinMode(MS3, OUTPUT);

	digitalWrite(MS1, HIGH);  // 1/16 microstep mode
	digitalWrite(MS2, HIGH);
	digitalWrite(MS3, HIGH);
	digitalWrite(ENABLE_PIN, LOW); // Enable the driver
	Serial.begin(74880);
	Serial.println("A4989 Stepper Motor Test - Wemos Lolin D1 Mini");
}

void stepMotor(int steps, int delayTime) {
	for (int i = 0; i < steps; i++) {
		digitalWrite(STEP_PIN, HIGH);
		delayMicroseconds(delayTime);
		digitalWrite(STEP_PIN, LOW);
		delayMicroseconds(delayTime);
	}
}

void loop() {
	// Forward Rotation
	digitalWrite(DIR_PIN, HIGH);
	stepMotor(stepsPerRevolution, 1000);  // Adjust speed with delay
	delay(1000);

	// Reverse Rotation
	digitalWrite(DIR_PIN, LOW);
	stepMotor(stepsPerRevolution, 1000);
	delay(1000);
}
