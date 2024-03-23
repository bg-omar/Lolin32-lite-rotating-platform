
#include "ezButton.h"

ezButton::ezButton(int pin): ezButton(pin, INPUT_PULLUP) {};

ezButton::ezButton(int pin, int mode) {
	btnPin = pin;
	debounceTime = 0;
	count = 0;
	countMode = COUNT_FALLING;

	pinMode(btnPin, mode);

	previousSteadyState = digitalRead(btnPin);
	lastSteadyState = previousSteadyState;
	lastFlickerableState = previousSteadyState;

	lastDebounceTime = 0;
}

void ezButton::setDebounceTime(unsigned long time) {
	debounceTime = time;
}

int ezButton::getState(void) {
	return lastSteadyState;
}

int ezButton::getStateRaw(void) {
	return digitalRead(btnPin);
}

bool ezButton::isPressed(void) {
	if(previousSteadyState == HIGH && lastSteadyState == LOW)
		return true;
	else
		return false;
}

bool ezButton::isReleased(void) {
	if(previousSteadyState == LOW && lastSteadyState == HIGH)
		return true;
	else
		return false;
}

void ezButton::setCountMode(int mode) {
	countMode = mode;
}

unsigned long ezButton::getCount(void) {
	return count;
}

void ezButton::resetCount(void) {
	count = 0;
}

void ezButton::loop(void) {
	// read the state of the switch/button:
	int currentState = digitalRead(btnPin);
	unsigned long currentTime = millis();

	// check to see if you just pressed the button
	// (i.e. the input went from LOW to HIGH), and you've waited long enough
	// since the last press to ignore any noise:

	// If the switch/button changed, due to noise or pressing:
	if (currentState != lastFlickerableState) {
		// reset the debouncing timer
		lastDebounceTime = currentTime;
		// save the the last flickerable state
		lastFlickerableState = currentState;
	}

	if ((currentTime - lastDebounceTime) >= debounceTime) {
		// whatever the reading is at, it's been there for longer than the debounce
		// delay, so take it as the actual current state:

		// save the the steady state
		previousSteadyState = lastSteadyState;
		lastSteadyState = currentState;
	}

	if(previousSteadyState != lastSteadyState){
		if(countMode == COUNT_BOTH)
			count++;
		else if(countMode == COUNT_FALLING){
			if(previousSteadyState == HIGH && lastSteadyState == LOW)
				count++;
		}
		else if(countMode == COUNT_RISING){
			if(previousSteadyState == LOW && lastSteadyState == HIGH)
				count++;
		}
	}
}
