//
// AHT10 Temperature and Humidity Sensor
//

#ifndef ARDUINO_R4_UNO_WALL_Z_AHT10_H
#define ARDUINO_R4_UNO_WALL_Z_AHT10_H

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_AHTX0.h>

class aht10 {
private:

public:
    static float temperature;
    static float humidity;
    
    static float readTemperature();
    static float readHumidity();
    static void showAHT10();
    static void aht10Setup();
    static void displaySensorDetails();

    static Adafruit_AHTX0 *aht;
};

#endif //ARDUINO_R4_UNO_WALL_Z_AHT10_H

