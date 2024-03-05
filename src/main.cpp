#include <Arduino.h>
#include "main.h"
#include "motor.h"

int direction = 0;
int speed = 3;

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    int i;
    for(i=0;i<4;i++){
        LOG("Setup Pin: ");
        LOGL(motorPins[i]);
        pinMode(motorPins[i], OUTPUT);
    }

}

void loop() {
    motor::moveSteps(direction, speed, 512);
    delay(50);
}

