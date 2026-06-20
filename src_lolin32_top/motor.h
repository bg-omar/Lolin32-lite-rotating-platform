//
// Created by mr on 12/21/2024.
//

#ifndef ESP8266_MAGIC_MOTOR_H
#define ESP8266_MAGIC_MOTOR_H

#include "main.h"
#include <cstdint>

class motor {
public:
    static int motorPinA;
    static int motorPinB; // set to -1 if unused

    static inline void write(uint8_t duty) {
        main::analogWriteESP32(motorPinA, duty);
        if (motorPinB >= 0) main::analogWriteESP32(motorPinB, duty);
    }

    static void rampPulseWithPower();
    static void alternatingPulseWithPower();
    static void quickBurstWithPower();
    static void pulsateWithPower();
    static void rotateWithPower();
    static void alternatingPulse();
    static void longPulse();
    static void shortPulse();
    static void randomVibrationSequence(int durationMs);

    static void customSequence(int pattern[], int size);

    static void sequence();
};

#endif // ESP8266_MAGIC_MOTOR_H