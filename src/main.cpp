#include <Arduino.h>
#include <Stepper.h>
#include <AccelStepper.h>

#define POTENTIOMETER 34

const int stepsPerRevolution = 200;

// initialize the stepper library on pins 8 through 11:
Stepper myStepper(stepsPerRevolution, 32, 33, 25, 26);
//AccelStepper stepper(4,  32, 33, 25, 26);
int stepCount = 0;  // number of steps the motor has taken
#define ANALOG_IN 34

int dir = 0;
void setup()
{
	//Serial.begin(9600);
	//Serial.println("ESP32 Water bottom plate Rotating Version: 0.0.1 ");
	// set the speed at 60 rpm:
	myStepper.setSpeed(60);
	pinMode(POTENTIOMETER,INPUT_PULLUP);
//	stepper.setMaxSpeed(60);
//	stepper.setSpeed(30);
//	stepper.setAcceleration(20);
//	stepper.moveTo(500);
}

void loop()
{

	//stepper.runSpeed();
	int analog_in = analogRead(ANALOG_IN);
	//Serial.println(analog_in);

	int AnalIn;
	if(analog_in < 3350){
		dir = 0;
		AnalIn = map(analog_in, 0, 3350, 30, 0);
	} else {
		dir = 1;
		AnalIn = map(analog_in, 3350, 4095, 0, 30);
	}


	if (AnalIn > 0) {
		myStepper.setSpeed(AnalIn);
		// step 1/100 of a revolution:
		myStepper.step((dir == 1 ? stepsPerRevolution : -stepsPerRevolution)/ 100);
	}
}
