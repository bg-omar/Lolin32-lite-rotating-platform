//
// 0.91" OLED Display (128x32) - Wide Screen
// Used for displaying sensor data and text
//

#ifndef ARDUINO_R4_UNO_WALL_Z_OLED91_H
#define ARDUINO_R4_UNO_WALL_Z_OLED91_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class oled91 {
private:

public:
    static void oled91Setup();
    static void clear();
    static void showText(int line, const char* text);
    static void showSensorData();
    static void update();
    
    static Adafruit_SSD1306 *oled;
    static bool initialized;
};

#endif //ARDUINO_R4_UNO_WALL_Z_OLED91_H

