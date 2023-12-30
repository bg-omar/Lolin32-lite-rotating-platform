#include <Arduino.h>
#include "main.h"
#include "motor.h"


#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile

RH_ASK driver(2000, 16, 17, 0); // ESP8266 or ESP32: do not use pin 11 or 2






int direction = 1;
int speed = 4;

void setup() {

    Serial.begin(115200);

    if (!driver.init()) Serial.println("init failed");

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
    motor::moveSteps(direction, speed, 512);
    const char *msg = "hello";
    Serial.println("SEND.....");
    driver.send((uint8_t *)msg, strlen(msg));
    driver.waitPacketSent();
    delay(200);
}

