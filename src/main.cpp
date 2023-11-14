#include <Arduino.h>
#include <ezButton.h>
#include <stdio.h>

const int motorPins[]={32,33,25,26};    //define pins connected to four phase ABCD of stepper motor
const int CCWStep[]={0x01,0x02,0x04,0x08};  //define power supply order for coil for rotating anticlockwise
const int CWStep[]={0x08,0x04,0x02,0x01};   //define power supply order for coil for rotating clockwise

#define LED_BUILTIN 22
#define powerPin 2
#define buttonUpPin 15
#define buttonDownPin 13

bool power = false;
int direction = 0;
int speed = 3;
ezButton powerButton(powerPin);
ezButton buttonUp(buttonUpPin);
ezButton buttonDown(buttonDownPin);


//as for four phase stepping motor, four steps is a cycle. the function is used to drive the stepping motor clockwise or anticlockwise to take four steps
void moveOnePeriod(int dir,int ms){
    int i=0,j=0;
    for (j=0;j<4;j++){  //cycle according to power supply order
        for (i=0;i<4;i++){  //assign to each pin, a total of 4 pins
            if(dir == 1)
                digitalWrite(motorPins[i],(CCWStep[j] == (1<<i)) ? HIGH : LOW);
            else
                digitalWrite(motorPins[i],(CWStep[j] == (1<<i)) ? HIGH : LOW);
            //Serial.println("motorPin %d, %d \n" + motorPins[i] + digitalRead(motorPins[i]));
        }
        //Serial.println("Step cycle!\n");
        if(ms<3)        //the delay can not be less than 3ms, otherwise it will exceed speed limit of the motor
            ms=3;
        delay(ms);
    }
}
//continuous rotation function, the parameter steps specifies the rotation cycles, every four steps is a cycle
void moveSteps(int dir, int ms, int steps){
    int i;
    for(i=0;i<steps;i++){
        moveOnePeriod(dir,ms);
    }
}
void motorStop(){   //function used to stop rotating
    int i;
    for(i=0;i<4;i++){
        digitalWrite(motorPins[i],LOW);
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("Serial started");
    pinMode(LED_BUILTIN, OUTPUT);
    powerButton.setDebounceTime(20); // set debounce time to 50 milliseconds
    buttonUp.setDebounceTime(20); // set debounce time to 50 milliseconds
    buttonDown.setDebounceTime(20); // set debounce time to 50 milliseconds

    int i;
    for(i=0;i<4;i++){
        Serial.print("Setup Pin: ");
        Serial.println(motorPins[i]);
        pinMode(motorPins[i], OUTPUT);
    }
}

void loop() {
    powerButton.loop();
    buttonUp.loop(); // MUST call the loop() function first
    buttonDown.loop(); // MUST call the loop() function first
    Serial.println(speed);

    if(buttonDown.isPressed()) {
        Serial.print("The buttonDown is pressed");
        speed++;
        if(speed>10)        //the delay can not be less than 3ms, otherwise it will exceed speed limit of the motor
            speed=10;
    }

    if(buttonUp.isPressed()) {
        Serial.println("The buttonUp is pressed");
        speed--;
        if(speed<3)        //the delay can not be less than 3ms, otherwise it will exceed speed limit of the motor
            speed=3;
    }

    if (powerButton.isPressed()) {
        Serial.println("The power / direction button");
        if (power){
            power = false;
            motorStop();
        } else {
            power = true;
            if (direction == 0) {
                direction = 1;
            } else { direction = 0; }
        }
    }

    if (power) {
        digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
        moveSteps(direction, speed, 1);     //rotating
    }
//    if (toggleSwitch.isReleased())
//        Serial.println("The switch: ON -> OFF");
//    int state = toggleSwitch.getState();
//    if (state == HIGH)
//        Serial.println("The switch: OFF");
//    else
//        Serial.println("The switch: ON");
//

}

