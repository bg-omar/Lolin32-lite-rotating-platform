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

    static inline void write(int duty) {
        duty = constrain(duty, 0, MOTOR_PWM_MAX);
        main::analogWriteESP32(motorPinA, duty);
        if (motorPinB >= 0) main::analogWriteESP32(motorPinB, duty);
    }

    static void rampPulseWithPower();
    static void alternatingPulseWithPower();
    static void runPreconditionThenPulse(const char* testName, unsigned long preMs, unsigned long pulseMs, unsigned long postMs, int deltaPwm,
                                  bool negativePulse);
    static void runNoPreconditionPulseControl(const char* testName, unsigned long idleMs, unsigned long pulseMs, unsigned long postMs, int deltaPwm);
    static void runSteadyOnlyTest(const char* testName, unsigned long holdMs);
    static void runMemoryDecayAfterStop(const char* testName, unsigned long preMs, unsigned long stopMs, unsigned long pulseMs, unsigned long postMs,
                                 int deltaPwm);
    static void quickBurstWithPower();
    static void pulsateWithPower();
    static void rotateWithPower();
    static void alternatingPulse();
    static void longPulse();
    static void shortPulse();
    static void randomVibrationSequence(int durationMs);

    static void customSequence(int pattern[], int size);

    static void controlPulseNoPreSpin();
    static void preconditionPulseTest10s();
    static void preconditionPulseTest30s();
    static void preconditionPulseTest60s();
    static void preconditionPulseTest120s();
    static void steadyRotorOnlyTest60s();
    static void memoryDecayAfterStopTest();
    static void negativePulseAfterPreconditionTest();

    static void sequence();
};

#endif // ESP8266_MAGIC_MOTOR_H