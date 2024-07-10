/********************************************** Setup booting the arduino **************************************/
// section Includes
/***************************************************************************************************************/

#include <Arduino.h>
#include <ESP32Servo.h>

#define LED_BUILTIN 22
#define POTENTIOMETER 34
#define SERVO_PIN 12

/********************************************** Setup booting the arduino **************************************/
// section Variables & Defines
/***************************************************************************************************************/

Servo myservo;  // create servo object
int posx = 93;    // variable to store the servo position
int potValue = 0;
/********************************************** Setup booting the arduino **************************************/
// section Functions
/***************************************************************************************************************/


void send(const String texting) {
    Serial.println(texting);
}

void sendServo(int pos) {
    Serial.println(pos);
    myservo.write(pos);
}
/********************************************** Setup booting the arduino **************************************/
// section Setup
/***************************************************************************************************************/

void setup() {
    Serial.begin(115200);
    Serial.println("ESP32 Water bottom plate Rotating Version: 0.0.1 ");
    myservo.setPeriodHertz(50);    // standard 50 hz servo
    myservo.attach(SERVO_PIN);
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(POTENTIOMETER,INPUT_PULLUP);
    delay(500);
}

void loop() {
    potValue = analogRead(POTENTIOMETER);
    Serial.println(potValue);
    if (potValue <= 3600) posx = map(potValue, 0, 3600, 0, 90); // 3600 is middle
    if (potValue > 3600)  posx = map(potValue, 3601, 4095, 91, 180);
    if (posx > 93 && posx < 97) posx = 87;
    if (posx > 88 && posx < 93) posx = 98;
    sendServo(posx);
    delay(75);
}


