//
// Created by mr on 11/17/2023.
//
// Compass (HMC5883) Sensor
// Orientation: Z-axis vertical, XY plane level/flat
// - X and Y axes are horizontal (level)
// - Z axis is vertical (pointing up/down)
// Heading calculation uses X and Y components for horizontal direction

#include "compass.h"

#include "main.h"



#include <Arduino.h>

#include <Wire.h> // library required for IIC communication

Adafruit_HMC5883_Unified* compass::mag = nullptr;
float DECLINATION_ANGLE = 0.035;

void compass::compassSetup() {
	if (!main::Found_I2C) return;
	compass::mag = new Adafruit_HMC5883_Unified(12345);
	if (!compass::mag->begin()) {
		LOGL("HMC5883 Compass not found");
		main::Found_Compass = false;
		delay(500);
	} else {
		LOGL("Compass Found!");
		main::Found_Compass = true;
		compass::displaySensorDetails();
		delay(500);
	}

	// Default setup for compass can be added here if needed
}

void compass::showCompass(){
    if (!main::logCompass) return;

    double headingDegrees = readCompass();
    if (isnan(headingDegrees)) {
        LOGL("Compass read failed");
        return;
    }

	LOG("Compass ");
    char buffer[20];

    snprintf(buffer, sizeof(buffer), "%f", headingDegrees);

	LOG(buffer);
    if (headingDegrees >= 0 && headingDegrees < 45){
		LOG("  North  ");
    }
    if (headingDegrees >= 45 && headingDegrees < 135){
		LOG("  East  ");
    }
    if (headingDegrees >= 135 && headingDegrees < 225){
		LOG("  South   ");
    }
    if (headingDegrees >= 225 && headingDegrees < 315){
		LOG("  West  ");
    }
    if (headingDegrees >= 315 && headingDegrees < 360){
		LOG("  North ");
    }
    LOGL("");
}

double compass::readCompass(){
    if (!mag || !main::Found_Compass) {
        return NAN; // or a sentinel like -1
    }
    sensors_event_t event; /// Get a new sensor event */
    mag->getEvent(&event);

	/* magnetic vector values are in micro-Tesla (uT) — serial log only when enabled */
	if (main::logCompass) {
		LOG("X: "); LOGD(((event.magnetic.x))); LOG("  ");
		LOG("Y: "); LOGD(((event.magnetic.y))); LOG("  ");
		LOG("Z: "); LOGD(((event.magnetic.z))); LOG("  "); LOGL("uT");
	}

	// Calculate heading using horizontal X and Y components (Z-axis vertical orientation)
	double heading = atan2(event.magnetic.y, event.magnetic.x) + DECLINATION_ANGLE;

    if(heading < 0) {
        heading += 2 * PI;
    }
    if(heading > 2*PI) {
        heading -= 2 * PI;
    }
    double headingDegrees = (heading * 180/M_PI) - 90;
	if (main::logCompass) {
		LOG("Heading (degrees): "); LOGLD(((headingDegrees)));
	}
    return (headingDegrees < 0) ? 360 + headingDegrees : headingDegrees;
}

void compass::displaySensorDetails(){
    sensor_t sensor;
    mag->getSensor(&sensor);
    Serial.println("------------------------------------");
    Serial.print  ("Sensor:       "); Serial.println(sensor.name);
    Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
    Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
    Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" uT");
    Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" uT");
    Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" uT");

    sensors_event_t event; /// Get a new sensor event */
    mag->getEvent(&event);

    Serial.print("X: "); Serial.print(event.magnetic.x); Serial.print("  ");
    Serial.print("Y: "); Serial.print(event.magnetic.y); Serial.print("  ");
    Serial.print("Z: "); Serial.print(event.magnetic.z); Serial.print("  ");Serial.println("  ");
    Serial.println("------------------------------------");
    Serial.println("");
    delay(500);
}



char buffer[20]; // helper buffer for displaying strings on the display