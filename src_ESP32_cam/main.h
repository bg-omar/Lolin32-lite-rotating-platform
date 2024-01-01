//
// Created by mr on 11/14/2023.
//

#ifndef ARDUINO_ESP_32_CAM_MAIN_H
#define ARDUINO_ESP_32_CAM_MAIN_H


#include <cstdint>

#define LED_BUILTIN 22

#define LOGL Serial.println
#define LOG Serial.print


/********************************************** Setup booting the arduino **************************************/
// section Variables & Defines
/***************************************************************************************************************/

#define EVENTS 0
#define BUTTONS 0
#define JOYSTICKS 0
#define SENSORS 0


bool power = true;
int direction = 0;
int speed = 3;





#endif //ARDUINO_ESP_32_CAM_MAIN_H
