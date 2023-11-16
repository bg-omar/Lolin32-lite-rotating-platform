#include <Arduino.h>
#include "main.h"
#include "motor.h"
#include <AccelStepper.h>


#define MotorInterfaceType 4
AccelStepper myStepper(MotorInterfaceType,32,33,25,26);

__attribute__((unused)) void setup() {
    Serial.begin(115200);
    LOGL("Serial started");
    myStepper.setMaxSpeed(1000.0);
    myStepper.setAcceleration(50.0);
    myStepper.setSpeed(200);
    myStepper.moveTo(2038);

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(powerPin, INPUT);
    pinMode(buttonUpPin, INPUT);
    pinMode(buttonDownPin, INPUT);

    int i;
    for(i=0;i<4;i++){
        LOG(motorPins[i]);
        pinMode(motorPins[i], OUTPUT);
    }
}

void loop() {

    if (digitalRead(buttonUpPin) == HIGH) {
        speed++;
        if (speed > 10) speed = 10;
        LOG(*(&"Down is pressed: "+speed));
    }

    if (digitalRead(buttonDownPin) == HIGH) {
        speed--;
        if (speed < 3) speed = 3;
        LOG(*(&"Up is pressed: "+speed));
    }

    if (digitalRead(powerPin) == HIGH) {
        if (power) {
            power = false;
            motor::motorStop();
            LOGL("Power off");
        } else {
            power = true;
            if (direction == 0) {
                LOGL("Rotating CW (1)");
                direction = 1;
            } else {
                LOGL("Rotating CCW (0)");
                direction = 0;
            }
        }
    }
    if (power) {
        digitalWrite(LED_BUILTIN, HIGH);
        motor::moveSteps(direction, speed, 32);
    } else {
        myStepper.run();
    }
}

