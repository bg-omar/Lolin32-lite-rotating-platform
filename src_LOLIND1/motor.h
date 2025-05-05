//
// Created by mr on 12/21/2024.
//

#ifndef ESP8266_MAGIC_MOTOR_H
#define ESP8266_MAGIC_MOTOR_H

#include "main.h"
#include <cstdlib>

class motor {
public:
	static void rampPulseWithPower(int power);
	static void alternatingPulseWithPower(int power);
	static void quickBurstWithPower(int power);
	static void pulsateWithPower(int power);
	static void rotateWithPower(int power);
	static void alternatingPulse(int power);
	static void longPulse(int power);
	static void shortPulse(int power);
	static void randomVibrationSequence(int maxPower, int durationMs);

	static void customSequence(int pattern[], int i, int power);
};


#endif //ESP8266_MAGIC_MOTOR_H
