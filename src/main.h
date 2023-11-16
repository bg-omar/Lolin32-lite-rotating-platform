//
// Created by mr on 11/15/2023.
//

#ifndef ARDUINO_WEMOS_LOLIN32_LITE_MAIN_H
#define ARDUINO_WEMOS_LOLIN32_LITE_MAIN_H

#define LED_BUILTIN 22
#define powerPin 2
#define buttonUpPin 15
#define buttonDownPin 13
#define LOGL Serial.println
#define LOG Serial.print


bool power = true;
int direction = 0;
int speed = 3;

__attribute__((unused)) void setup();
void loop();

#endif //ARDUINO_WEMOS_LOLIN32_LITE_MAIN_H
