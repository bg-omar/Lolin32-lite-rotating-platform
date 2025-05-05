#include "motor.h"


void motor::randomVibrationSequence(int maxPower, int durationMs) {
	static unsigned long sequenceStart = millis();
	static unsigned long lastUpdate = millis();
	static bool isOn = false;
	static int pulseCount = 0;
	static int currentPower = 0;

	if (millis() - sequenceStart >= durationMs) {
		analogWrite(motorPin, 0); // Turn off the motor after sequence ends
		return;
	}

	// Randomized intervals for on/off switching
	unsigned long randomInterval = random(50, 300); // 50ms to 300ms intervals

	if (millis() - lastUpdate >= randomInterval) {
		lastUpdate = millis();

		if (isOn) {
			analogWrite(motorPin, 0); // Turn off motor
			isOn = false;
		} else {
			// Random power level and pulse behavior
			currentPower = random(maxPower / 2, maxPower);
			analogWrite(motorPin, currentPower);
			isOn = true;

			// Add randomness in the type of vibration
			int pattern = random(0, 3); // Randomly pick a vibration pattern
			switch (pattern) {
				case 0:
					shortPulse(currentPower);
					break;
				case 1:
					longPulse(currentPower);
					break;
				case 2:
					alternatingPulse(currentPower);
					break;
			}
		}
	}
}



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



void motor::alternatingPulse(int power) {
	static unsigned long lastUpdate = 0;
	static bool highPower = true;
	static int pulseCount = 0;
	static int totalPulses = random(3, 6); // Randomize number of alternations

	if (millis() - lastUpdate >= random(50, 150)) { // Random high/low duration
		lastUpdate = millis();

		if (pulseCount < totalPulses) {
			analogWrite(motorPin, highPower ? power : power / 2); // Alternate power levels
			highPower = !highPower;

			if (!highPower) { // Increment pulse count after a full high-low cycle
				pulseCount++;
			}
		} else {
			analogWrite(motorPin, 0); // Turn off motor after pulses are complete
			pulseCount = 0; // Reset pulse count
			totalPulses = random(3, 6); // Generate new random pulse count
		}
	}
}

void motor::customSequence(int pattern[], int size, int power) {
	static int index = 0;
	static unsigned long lastUpdate = 0;

	if (millis() - lastUpdate >= pattern[index]) {
		lastUpdate = millis();

		// Alternate between on and off states
		analogWrite(motorPin, index % 2 == 0 ? power : 0);

		index = (index + 1) % size; // Move to the next pattern duration
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
