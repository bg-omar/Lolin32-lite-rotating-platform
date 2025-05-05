//
// Created by mr on 11/15/2023.
//

#ifndef ARDUINO_WEMOS_LOLIN32_LITE_MAIN_H
#define ARDUINO_WEMOS_LOLIN32_LITE_MAIN_H

#define LOGL main::logln
#define LOG main::log
#define LOGLI main::logIntln
#define LOGI main::logInt
#define LOGLF main::logFloatln
#define LOGF main::logFloat
#define LOGLD main::logDoubbleln
#define LOGD main::logDoubble


#define VERBOSE 0
#define PS4CONTROLLER 1
#define EVENTS 0
#define BUTTONS 0
#define JOYSTICKS 0
#define SENSORS 0
#define THUMB_STICKS 0
#define  USE_I2C_SCANNER 1
#include <Arduino.h>

const int motorPin = 14;  // PWM-capable pin on ESP8266
const int led = LED_BUILTIN;
#define THRESHOLD 20

class main {

public:
	static void printPSI_I2O();

	static bool Found_Gyro;
	static bool Found_Compass;
	static bool Found_Barometer;

	static void logln(const char *text);

	static void log(const char *text);

	static void logDoubble(double floaty);

	static void logDoubbleln(double floaty);

	static void logFloat(float floaty);

	static void logFloatln(float floaty);

	static void logInt(int inty);

	static void logIntln(int inty);

	void logHex(unsigned char hexy, int i);

	void logHexln(unsigned char hexy, int i);
};

#endif //ARDUINO_WEMOS_LOLIN32_LITE_MAIN_H
