//
// Created by mr on 11/14/2023.
//

#ifndef ARDUINO_ESP_32_CAM_MAIN_H
#define ARDUINO_ESP_32_CAM_MAIN_H


#include <cstdint>

#define LED_BUILTIN 22

#define LOGL Serial.println
#define LOG Serial.print

#define USE_GET 0
#define USE_SERVER 0

//#define RH_speed  2000
//#define rxPin 3
//#define txPin 1
//#define pttPin  10
//#define pttInverted 0

bool power = true;
int direction = 0;
int speed = 3;





#endif //ARDUINO_ESP_32_CAM_MAIN_H
