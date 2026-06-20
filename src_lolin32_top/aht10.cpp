//
// AHT10 Temperature and Humidity Sensor
//

#include "aht10.h"
#include "main.h"

Adafruit_AHTX0* aht10::aht = nullptr;
float aht10::temperature = 0.0;
float aht10::humidity = 0.0;

void aht10::aht10Setup() {
    aht10::aht = new Adafruit_AHTX0();
    if (!aht10::aht->begin()) {
        LOGL("AHT10 not found");
        delay(500);
        main::Found_AHT10 = false;
    } else {
        LOGL("AHT10 Found!");
        main::Found_AHT10 = true;
        aht10::displaySensorDetails();
        delay(500);
    }
}

void aht10::showAHT10() {
    if (!aht || !main::Found_AHT10) {
        return;
    }
    
    sensors_event_t humidity_event, temp_event;
    if (!aht->getEvent(&humidity_event, &temp_event)) {
        LOGL("AHT10 read failed");
        return;
    }

    aht10::temperature = temp_event.temperature;
    aht10::humidity = humidity_event.relative_humidity;

    LOG("Temperature: ");
    LOGF(temperature);
    LOG("°C  Humidity: ");
    LOGF(humidity);
    LOGL("%");
}

float aht10::readTemperature() {
    if (!aht || !main::Found_AHT10) {
        return NAN;
    }
    
    sensors_event_t humidity_event, temp_event;
    if (!aht->getEvent(&humidity_event, &temp_event)) {
        return NAN;
    }
    
    aht10::temperature = temp_event.temperature;
    return temperature;
}

float aht10::readHumidity() {
    if (!aht || !main::Found_AHT10) {
        return NAN;
    }
    
    sensors_event_t humidity_event, temp_event;
    if (!aht->getEvent(&humidity_event, &temp_event)) {
        return NAN;
    }
    
    aht10::humidity = humidity_event.relative_humidity;
    return humidity;
}

void aht10::displaySensorDetails() {
    if (!aht || !main::Found_AHT10) {
        return;
    }
    
    Serial.println("------------------------------------");
    Serial.println("AHT10/AHT20 Sensor");
    Serial.println("------------------------------------");
    
    // Read and display current values
    sensors_event_t humidity_event, temp_event;
    if (aht->getEvent(&humidity_event, &temp_event)) {
        Serial.print("Temperature: ");
        Serial.print(temp_event.temperature);
        Serial.println(" °C");
        Serial.print("Humidity: ");
        Serial.print(humidity_event.relative_humidity);
        Serial.println(" %");
    } else {
        Serial.println("Failed to read sensor data");
    }
    
    Serial.println("------------------------------------");
    Serial.println("");
    delay(500);
}

