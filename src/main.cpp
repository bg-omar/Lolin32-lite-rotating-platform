#include <Arduino.h>
#include "ezButton.h"
#include "main.h"
#include "motor.h"


ezButton powerButton(powerPin);
ezButton buttonUp(buttonUpPin);
ezButton buttonDown(buttonDownPin);

void setup() {
    Serial.begin(115200);
    LOGL("Serial started");
    pinMode(LED_BUILTIN, OUTPUT);
    powerButton.setDebounceTime(50); // set debounce time to 50 milliseconds
    buttonUp.setDebounceTime(50); // set debounce time to 50 milliseconds
    buttonDown.setDebounceTime(50); // set debounce time to 50 milliseconds

    LOG(direction);
    int i;
    for(i=0;i<4;i++){
        LOG("Setup Pin: ");
        LOGL(motorPins[i]);
        pinMode(motorPins[i], OUTPUT);
    }
}

void loop() {
    powerButton.loop();
    buttonUp.loop(); // MUST call the loop() function first
    buttonDown.loop(); // MUST call the loop() function first

    if(buttonDown.isPressed()) {
        LOG("The buttonDown is pressed: ");
        speed++;
        if(speed>10)        //the delay can not be less than 3ms, otherwise it will exceed speed limit of the motor
            speed=10;
        LOGL(speed);
    }

    if(buttonUp.isPressed()) {
        LOGL("The buttonUp is pressed: ");
        speed--;
        if(speed<3)        //the delay can not be less than 3ms, otherwise it will exceed speed limit of the motor
            speed=3;
        LOGL(speed);
    }

    if (powerButton.isPressed()) {
        if (power){
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
        digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
        motor::moveSteps(direction, speed, 32);     //rotating
    }
//    if (toggleSwitch.isReleased())
//        LOGL("The switch: ON -> OFF");
//    int state = toggleSwitch.getState();
//    if (state == HIGH)
//        LOGL("The switch: OFF");
//    else
//        LOGL("The switch: ON");
//

}

