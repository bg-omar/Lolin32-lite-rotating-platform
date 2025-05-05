//
// Created by mr on 11/17/2023.
//

#include "compass.h"

#include "main.h"



#include <Arduino.h>

#include <Wire.h> // library required for IIC communication

Adafruit_HMC5883_Unified* compass::mag = nullptr;
float DECLINATION_ANGLE = 0.035;

void compass::compassSetup() {
	compass::mag = new Adafruit_HMC5883_Unified(12345);
	if (!compass::mag->begin()) {
		LOGL("HMC5883 Compass not found");
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
    unsigned char north[] =  {0x00,0x00,0x00,0x00,0x00,0x7e,0x04,0x08,0x10,0x20,0x7e,0x00,0x00,0x00,0x00,0x00};
    unsigned char east[]  =  {0x00,0x00,0x00,0x00,0x00,0xfe,0x92,0x92,0x92,0x92,0x82,0x00,0x00,0x00,0x00,0x00};
    unsigned char south[] =  {0x00,0x00,0x00,0x00,0x00,0x9e,0x92,0x92,0x92,0x92,0xf2,0x00,0x00,0x00,0x00,0x00};
    unsigned char west[]  =  {0x00,0x00,0x00,0x3c,0x40,0x40,0x40,0x78,0x78,0x40,0x40,0x40,0x3c,0x00,0x00,0x00};

    double headingDegrees = readCompass();
	LOG("Compass ");
    char buffer[20]; // Assuming a buffer size of 20 is sufficient

    // Convert double to char*
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
}

double compass::readCompass(){
    sensors_event_t event; /// Get a new sensor event */
    mag->getEvent(&event);

	/* Display the results (magnetic vector values are in micro-Tesla (uT)) */
	LOG("X: "); LOGD(((event.magnetic.x))); LOG("  ");
	LOG("Y: "); LOGD(((event.magnetic.y))); LOG("  ");
	LOG("Z: "); LOGD(((event.magnetic.z))); LOG("  ");LOGL("uT");

	double heading = atan2(event.magnetic.y, event.magnetic.x) + DECLINATION_ANGLE;

    if(heading < 0) {
        heading += 2 * PI;
    }
    if(heading > 2*PI) {
        heading -= 2 * PI;
    }
    double headingDegrees = (heading * 180/M_PI) - 90;
	LOG("Heading (degrees): "); LOGLD(((headingDegrees)));
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
