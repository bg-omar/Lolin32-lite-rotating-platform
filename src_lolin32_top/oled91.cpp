//
// 0.91" OLED Display (128x32) - Wide Screen
// Used for displaying sensor data and text
//

#include "oled91.h"
#include "main.h"
#include "compass.h"
#include "gyroscope.h"
#include "aht10.h"
#include "bmp280.h"

Adafruit_SSD1306* oled91::oled = nullptr;
bool oled91::initialized = false;

void oled91::oled91Setup() {
    if (!main::Found_I2C) return;
    // 0.91" OLED is typically 128x32 pixels
    // Common I2C addresses: 0x3C (default) or 0x3D
    uint8_t addresses[] = {0x3C, 0x3D};
    bool found = false;
    
    for (uint8_t i = 0; i < 2; i++) {
        oled91::oled = new Adafruit_SSD1306(128, 32, &Wire, -1);
        if (oled91::oled->begin(SSD1306_SWITCHCAPVCC, addresses[i])) {
            LOG("0.91\" OLED Found at address 0x");
            char addrBuf[4];
            snprintf(addrBuf, sizeof(addrBuf), "%02X", addresses[i]);
            LOGL(addrBuf);
            main::Found_Display = true;
            oled91::initialized = true;
            found = true;
            
            // Initialize display
            oled91::oled->clearDisplay();
            oled91::oled->setTextSize(1);
            oled91::oled->setTextColor(SSD1306_WHITE);
            oled91::oled->setCursor(0, 0);
            oled91::oled->println("OLED Ready");
            oled91::oled->display();
            delay(500);
            break;
        }
        delete oled91::oled;
        oled91::oled = nullptr;
    }
    
    if (!found) {
        LOGL("0.91\" OLED not found at 0x3C or 0x3D");
        delay(500);
        main::Found_Display = false;
        oled91::initialized = false;
    }
}

void oled91::clear() {
    if (!oled || !initialized) {
        return;
    }
    oled->clearDisplay();
    oled->setCursor(0, 0);
}

void oled91::showText(int line, const char* text) {
    if (!oled || !initialized) {
        return;
    }
    
    // 0.91" OLED has 4 lines of text (8 pixels per line, 32 pixels total)
    // Line 0: y=0, Line 1: y=8, Line 2: y=16, Line 3: y=24
    int yPos = line * 8;
    if (yPos >= 32) yPos = 24; // Clamp to last line
    
    oled->setCursor(0, yPos);
    oled->setTextSize(1);
    oled->setTextColor(SSD1306_WHITE);
    oled->print(text);
}

void oled91::showSensorData() {
    if (!oled || !initialized) {
        return;
    }
    
    oled->clearDisplay();
    oled->setTextSize(1);
    oled->setTextColor(SSD1306_WHITE);
    
    int line = 0;
    char buffer[32];
    
    // Line 0: Compass heading
    if (main::Found_Compass) {
        double heading = compass::readCompass();
        if (!isnan(heading)) {
            snprintf(buffer, sizeof(buffer), "C:%.1f", heading);
            oled->setCursor(0, line * 8);
            oled->print(buffer);
        }
    }
    
    // Line 1: Temperature (AHT10 or BMP280)
    if (main::Found_AHT10) {
        float temp = aht10::readTemperature();
        if (!isnan(temp)) {
            snprintf(buffer, sizeof(buffer), "T:%.1fC H:%.1f%%", temp, aht10::readHumidity());
            oled->setCursor(0, (line + 1) * 8);
            oled->print(buffer);
        }
    } else if (main::Found_Barometer) {
        float temp = bmp280::readTemperature();
        if (!isnan(temp)) {
            snprintf(buffer, sizeof(buffer), "T:%.1fC P:%.0f", temp, bmp280::readPressure());
            oled->setCursor(0, (line + 1) * 8);
            oled->print(buffer);
        }
    }
    
    // Line 2: RPM and Centripetal Force (if gyro available)
    if (main::Found_Gyro) {
        gyroscope::gyroRead();
        gyroscope::calculateRPM();
        gyroscope::calculateCentripetalForce();
        snprintf(buffer, sizeof(buffer), "RPM:%.1f F:%.3fN", gyroscope::rpm, gyroscope::centripetalForce);
        oled->setCursor(0, (line + 2) * 8);
        oled->print(buffer);
    } else {
        // Fallback to gyro acceleration if no rotation data
        gyroscope::gyroRead();
        snprintf(buffer, sizeof(buffer), "AX:%.1f AY:%.1f", gyroscope::ax, gyroscope::ay);
        oled->setCursor(0, (line + 2) * 8);
        oled->print(buffer);
    }
    
    // Line 3: Gyro Z or Pressure/Altitude
    if (main::Found_Gyro) {
        snprintf(buffer, sizeof(buffer), "GZ:%.2f AZ:%.1f", gyroscope::gz, gyroscope::az);
        oled->setCursor(0, (line + 3) * 8);
        oled->print(buffer);
    } else if (main::Found_Barometer) {
        float alt = bmp280::readAltitude();
        if (!isnan(alt)) {
            snprintf(buffer, sizeof(buffer), "Alt:%.1fm", alt);
            oled->setCursor(0, (line + 3) * 8);
            oled->print(buffer);
        }
    }
    
    oled->display();
}

void oled91::update() {
    if (!oled || !initialized) {
        return;
    }
    oled->display();
}

