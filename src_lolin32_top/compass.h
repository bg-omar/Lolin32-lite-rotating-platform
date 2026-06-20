//
// Created by mr on 11/17/2023.
//
// Compass (HMC5883) Sensor Class
// Sensor Orientation: Z-axis vertical, XY plane level/flat
// - X and Y axes are horizontal (level)
// - Z axis is vertical (pointing up/down)
// Heading calculation uses X and Y components for horizontal direction

#ifndef ARDUINO_R4_UNO_WALL_Z_COMPASS_H
#define ARDUINO_R4_UNO_WALL_Z_COMPASS_H

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>

class compass {
private:

public:
    static double readCompass();
    static void showCompass();
    static void compassSetup() ;
    static void displaySensorDetails();

    static void displayCompass();

	static Adafruit_HMC5883_Unified *mag;
};



#endif //ARDUINO_R4_UNO_WALL_Z_COMPASS_H
