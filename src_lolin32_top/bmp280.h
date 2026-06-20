//
// BMP280/BME280 Barometric Pressure Sensor
// Supports both BMP280 (pressure only) and BME280 (pressure, temperature, humidity)
//

#ifndef ARDUINO_R4_UNO_WALL_Z_BMP280_H
#define ARDUINO_R4_UNO_WALL_Z_BMP280_H

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

class bmp280 {
private:

public:
    static float pressure;      // Pressure in hPa
    static float temperature;   // Temperature in °C
    static float altitude;      // Calculated altitude in meters
    static bool hasHumidity;     // True if BME280 (has humidity), false if BMP280
    
    static float readPressure();
    static float readTemperature();
    static float readAltitude(float seaLevelPressure = 1013.25);
    static float readHumidity();  // Only available on BME280
    static void showBMP280();
    static void bmp280Setup();
    static void displaySensorDetails();

    static Adafruit_BMP280 *bmp;
};

#endif //ARDUINO_R4_UNO_WALL_Z_BMP280_H

