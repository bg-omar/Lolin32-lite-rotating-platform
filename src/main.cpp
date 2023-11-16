#include <Arduino.h>
#include "main.h"
#include "motor.h"

bool power = true;
int direction = 0;
int speed = 3;

void setup() {
    Serial.begin(115200);
    LOGL("Serial started");
    pinMode(LED_BUILTIN, OUTPUT);

    int i;
    for(i=0;i<4;i++){
        LOG("Setup Pin: ");
        LOGL(motorPins[i]);
        pinMode(motorPins[i], OUTPUT);
    }
}

void loop() {
        digitalWrite(LED_BUILTIN, HIGH);
        motor::moveSteps(direction, speed, 512);
}

