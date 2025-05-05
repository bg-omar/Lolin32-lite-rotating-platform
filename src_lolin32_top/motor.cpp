#include "motor.h"


void motor::shortPulse(int power) {
	static unsigned long lastUpdate = 0;
	static bool isOn = false;

	if (millis() - lastUpdate >= (isOn ? random(50, 150) : 0)) {
		lastUpdate = millis();
		isOn = !isOn;

		if (isOn) {
			analogWrite(motorPin, power);
		} else {
			analogWrite(motorPin, 0);
		}
	}
}




void motor::longPulse(int power) {
	static unsigned long lastUpdate = 0;
	static bool isOn = false;

	if (millis() - lastUpdate >= (isOn ? random(300, 600) : 0)) {
		lastUpdate = millis();
		isOn = !isOn;

		if (isOn) {
			analogWrite(motorPin, power);
		} else {
			analogWrite(motorPin, 0);
		}
	}
}



void motor::rotateWithPower(int power) {
	static unsigned long lastUpdate = 0;
	static int speed = 0;
	static bool increasing = true;

	if (millis() - lastUpdate >= 20) { // Update every 20ms
		lastUpdate = millis();

		if (increasing) {
			speed += 5;
			if (speed >= power) {
				increasing = false; // Start decreasing
			}
		} else {
			speed -= 5;
			if (speed <= 0) {
				increasing = true; // Start increasing again
			}
		}

		analogWrite(motorPin, speed);
	}
}

void motor::pulsateWithPower(int power) {
	static unsigned long lastUpdate = 0;
	static bool isOn = false;
	static int pulseCount = 0;

	if (millis() - lastUpdate >= (isOn ? 200 : 100)) { // Toggle every 200ms
		lastUpdate = millis();
		isOn = !isOn;

		if (isOn) {
			analogWrite(motorPin, power);
			pulseCount++;
		} else {
			analogWrite(motorPin, 0);
		}

		if (pulseCount >= 5) { // Reset after 5 pulses
			pulseCount = 0;
		}
	}
}

void motor::quickBurstWithPower(int power) {
	static unsigned long lastUpdate = 0;
	static int state = 0;

	if (millis() - lastUpdate >= (state == 0 ? 300 : 100)) { // 300ms on, 100ms off
		lastUpdate = millis();

		if (state == 0) {
			analogWrite(motorPin, power); // Turn motor on
			state = 1;
		} else {
			analogWrite(motorPin, 0); // Turn motor off
			state = 0;
		}
	}
}

void motor::rampPulseWithPower(int power) {
	static unsigned long lastUpdate = 0;
	static int speed = 0;
	static int state = 0;

	if (millis() - lastUpdate >= 10) { // Update every 10ms
		lastUpdate = millis();

		if (state == 0) { // Ramp-up phase
			speed += 5;
			if (speed >= power) {
				state = 1; // Transition to hold phase
				lastUpdate = millis(); // Reset timer for holding
			}
		} else if (state == 1) { // Hold phase
			if (millis() - lastUpdate >= 200) { // Hold for 200ms
				state = 2; // Transition to ramp-down phase
			}
		} else if (state == 2) { // Ramp-down phase
			speed -= 5;
			if (speed <= 0) {
				state = 0; // Reset to ramp-up phase
				speed = 0;
			}
		}

		analogWrite(motorPin, speed);
	}
}

void motor::alternatingPulseWithPower(int power) {
	static unsigned long lastUpdate = 0;
	static bool highPower = true;
	static int pulseCount = 0;

	if (millis() - lastUpdate >= 150) { // Toggle every 150ms
		lastUpdate = millis();

		if (pulseCount < 5) { // Alternate between high and low power for 5 pulses
			analogWrite(motorPin, highPower ? power : power / 2);
			highPower = !highPower;
			if (!highPower) {
				pulseCount++; // Increment on each full cycle
			}
		} else {
			analogWrite(motorPin, 0); // Stop motor after 5 pulses
			pulseCount = 0; // Reset pulse count for the next cycle
		}
	}
}
