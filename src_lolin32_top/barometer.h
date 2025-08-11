//
// Created by mr on 11/20/2023.
//

#ifndef ARDUINO_R4_UNO_WALL_Z_BAROMETER_H
#define ARDUINO_R4_UNO_WALL_Z_BAROMETER_H

#include <Adafruit_BMP280.h>

class barometer {
public:
    static void baroSetup();
    static void baroMeter();

	static Adafruit_BMP280 *bmp;

	static void LOGHexln(uint8_t id, int i);
};


#endif //ARDUINO_R4_UNO_WALL_Z_BAROMETER_H
