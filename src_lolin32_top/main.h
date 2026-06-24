//
// Created by mr on 11/15/2023.
//

#ifndef ARDUINO_WEMOS_LOLIN32_LITE_MAIN_H
#define ARDUINO_WEMOS_LOLIN32_LITE_MAIN_H

#include <Arduino.h>
#define LOGL main::logln
#define LOG main::log
#define LOGLI main::logIntln
#define LOGI main::logInt
#define LOGLF main::logFloatln
#define LOGF main::logFloat
#define LOGLD main::logDoubbleln
#define LOGD main::logDoubble

#define SERVO 12        // Pin to control (pulse) servo
#define SERVO2 14 // Pin to middle tap of pot

#define MOTOR_PWM_MAX 1023   // 10-bit LEDC for motor

#define PRESET_PWM_CROSS     15
#define PRESET_PWM_SQUARE    40
#define PRESET_PWM_TRIANGLE  140

// 4 for flash led or 33 for normal led
#define VERBOSE 1
#define PS4CONTROLLER 1
#define EVENTS 1
#define BUTTONS 1
#define JOYSTICKS 0
#define SENSORS 0
#define THUMB_STICKS 1
#define  USE_I2C_SCANNER 1

#define I2C_SDA 4
#define I2C_SCL 5
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    (-1)
#define OLED_SDA I2C_SDA // 4
#define OLED_SCL I2C_SCL // 5

const int led = 22;
#define THRESHOLD 20

class main {
public:
    static void scan_PSI_I2C();

    static bool Found_Gyro;
    static bool Found_Compass;
    static bool Found_Barometer;
    static bool Found_AHT10;
    static bool Found_Display;
    static bool Found_I2C;

    static bool logCompass;

    static void analogWriteESP32(int pin, int value);

    static void logln(const char *text);
    static void log(const char *text);
    static void logDoubble(double floaty);
    static void logDoubbleln(double floaty);
    static void logFloat(float floaty);
    static void logFloatln(float floaty);
    static void logInt(int inty);
    static void logIntln(int inty);
    static void logHex(unsigned char hexy, int i);
    static void logHexln(unsigned char hexy, int i);

    // Motor control state
    static int motorPWM;
    static int motor2PWM;
    static int motorTimeMultiplyer;
    static int motorTimeFactor;        // NEW: clamped [1..20]
    static uint32_t motorSequenceEpoch; // NEW: increments on sequence changes to reset statics

    static int motorSequenceIndex;
    static int totalSequences;
    static bool motorSequence;
    static int motorBasePwmTarget;  // base PWM 0..MOTOR_PWM_MAX (presets + L1/R1/arrows)
};

#endif // ARDUINO_WEMOS_LOLIN32_LITE_MAIN_H