//
// Created by mr on 11/17/2023.
//
// Gyroscope/Accelerometer (MPU6050) Sensor Class
// Sensor Orientation: Z-axis vertical, XY plane level/flat
// - X and Y axes are horizontal (level)
// - Z axis is vertical (pointing up/down)
// - Acceleration: az should be ~9.8 m/s² (gravity) when level
// - Gyro: gx and gy detect horizontal rotation, gz detects vertical rotation

#ifndef ARDUINO_R4_UNO_WALL_Z_GYROSCOPE_H
#define ARDUINO_R4_UNO_WALL_Z_GYROSCOPE_H

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>


class gyroscope {
private:

    sensors_event_t a, gyro, temp;
public:
    static float ax,ay, az, gx, gy, gz, baseAx, baseAy, baseAz, baseGx, baseGy, baseGz, temperature;
    
    // Rotation calculations
    static float rpm;              // Revolutions per minute
    static float centripetalForce; // Centripetal force in Newtons
    static const float ROTATION_RADIUS; // Radius from center in meters (8 cm = 0.08 m)
    static const float DEVICE_MASS;     // Device mass in kg (default 0.1 kg = 100g)

    static bool gyroRead();  // Returns true on success, false on failure
    static void gyroFunc();
    static void gyroDetectMovement();
    static void showGyro();  // Display current acceleration and gyro values
    static float calculateRPM();  // Calculate RPM from angular velocity
    static float calculateCentripetalForce();  // Calculate centripetal force
    static void gyroCalibrate_sensor() ;
    static void gyroSetup() ;
};


#endif //ARDUINO_R4_UNO_WALL_Z_GYROSCOPE_H
