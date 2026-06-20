//
// BMP280/BME280 Barometric Pressure Sensor
// Supports both BMP280 (pressure only) and BME280 (pressure, temperature, humidity)
//

#include "bmp280.h"
#include "main.h"

Adafruit_BMP280* bmp280::bmp = nullptr;
float bmp280::pressure = 0.0;
float bmp280::temperature = 0.0;
float bmp280::altitude = 0.0;
bool bmp280::hasHumidity = false;

void bmp280::bmp280Setup() {
    // Try to initialize at common I2C addresses (0x76 is default, 0x77 is alternative)
    uint8_t addresses[] = {0x76, 0x77};
    bool found = false;
    
    for (uint8_t i = 0; i < 2; i++) {
        bmp280::bmp = new Adafruit_BMP280();
        if (bmp280::bmp->begin(addresses[i])) {
            LOG("BMP280/BME280 Found at address 0x");
            char addrBuf[4];
            snprintf(addrBuf, sizeof(addrBuf), "%02X", addresses[i]);
            LOGL(addrBuf);
            main::Found_Barometer = true;
            found = true;
            
            // Check if it's a BME280 (has humidity) or BMP280 (pressure only)
            // BME280 has chip ID 0x60, BMP280 has chip ID 0x58
            // The library handles both, but we can check sensor type
            // For now, we'll assume BME280 if humidity reading is available
            // Note: Adafruit_BMP280 library works with both sensors
            
            // Set sampling settings
            bmp280::bmp->setSampling(
                Adafruit_BMP280::MODE_NORMAL,      // Operating mode
                Adafruit_BMP280::SAMPLING_X2,      // Temperature oversampling
                Adafruit_BMP280::SAMPLING_X16,     // Pressure oversampling
                Adafruit_BMP280::FILTER_X16,       // Filtering
                Adafruit_BMP280::STANDBY_MS_500    // Standby time
            );
            
            bmp280::displaySensorDetails();
            delay(500);
            break;
        }
        delete bmp280::bmp;
        bmp280::bmp = nullptr;
    }
    
    if (!found) {
        LOGL("BMP280/BME280 not found at 0x76 or 0x77");
        delay(500);
        main::Found_Barometer = false;
    }
}

void bmp280::showBMP280() {
    if (!bmp || !main::Found_Barometer) {
        return;
    }
    
    bmp280::pressure = bmp280::readPressure();
    bmp280::temperature = bmp280::readTemperature();
    bmp280::altitude = bmp280::readAltitude();
    
    if (isnan(pressure) || isnan(temperature)) {
        LOGL("BMP280 read failed");
        return;
    }

    LOG("Pressure: ");
    LOGF(pressure);
    LOG(" hPa  Temperature: ");
    LOGF(temperature);
    LOG("°C  Altitude: ");
    LOGF(altitude);
    LOGL(" m");
    
    // Note: Humidity is only available on BME280, not BMP280
    // The Adafruit_BMP280 library doesn't expose humidity for BME280
    // If you need humidity, you'd need to use Adafruit_BME280 library instead
}

float bmp280::readPressure() {
    if (!bmp || !main::Found_Barometer) {
        return NAN;
    }
    
    float p = bmp->readPressure() / 100.0; // Convert Pa to hPa
    if (!isnan(p) && !isinf(p)) {
        bmp280::pressure = p;
    }
    return bmp280::pressure;
}

float bmp280::readTemperature() {
    if (!bmp || !main::Found_Barometer) {
        return NAN;
    }
    
    float t = bmp->readTemperature();
    if (!isnan(t) && !isinf(t)) {
        bmp280::temperature = t;
    }
    return bmp280::temperature;
}

float bmp280::readAltitude(float seaLevelPressure) {
    if (!bmp || !main::Found_Barometer) {
        return NAN;
    }
    
    float a = bmp->readAltitude(seaLevelPressure);
    if (!isnan(a) && !isinf(a)) {
        bmp280::altitude = a;
    }
    return bmp280::altitude;
}

float bmp280::readHumidity() {
    // Note: Adafruit_BMP280 library doesn't support humidity
    // For BME280 humidity support, you'd need Adafruit_BME280 library
    // This function returns NAN to indicate humidity is not available
    return NAN;
}

void bmp280::displaySensorDetails() {
    if (!bmp || !main::Found_Barometer) {
        return;
    }
    
    Serial.println("------------------------------------");
    Serial.println("BMP280/BME280 Sensor");
    Serial.println("------------------------------------");
    
    // Read and display current values
    float p = bmp->readPressure() / 100.0; // Convert Pa to hPa
    float t = bmp->readTemperature();
    float a = bmp->readAltitude();
    
    Serial.print("Pressure: ");
    Serial.print(p);
    Serial.println(" hPa");
    
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.println(" °C");
    
    Serial.print("Altitude: ");
    Serial.print(a);
    Serial.println(" m");
    
    Serial.println("------------------------------------");
    Serial.println("");
    delay(500);
}

