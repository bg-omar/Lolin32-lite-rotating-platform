#include "barometer.h"
#include <Wire.h>
#include "main.h"


// Define constants
constexpr float SEALEVELPRESSURE_HPA = 1013.25;  // Standard sea-level pressure

// Dynamic pointer for Adafruit_BMP280 to avoid static initialization issues
Adafruit_BMP280* barometer::bmp = nullptr;

void barometer::baroSetup() {
	// Initialize the barometer object
	barometer::bmp = new Adafruit_BMP280();

	if (!barometer::bmp->begin()) {
		#if VERBOSE
			LOGL("No valid BMP280 sensor, check wiring or try a different address!");
			LOG("SensorID was: 0x");
//			LOGLH(barometer::bmp->sensorID(), 16);
		#endif
		main::Found_Barometer = false;
		delete barometer::bmp;  // Clean up memory if initialization fails
		barometer::bmp = nullptr;
		return;
	} else {
		main::Found_Barometer = true;
	}

	LOGL("Barometer initialized successfully");

	// Default settings from datasheet
	barometer::bmp->setSampling(
			Adafruit_BMP280::MODE_FORCED,     /* Operating Mode */
			Adafruit_BMP280::SAMPLING_X2,     /* Temperature oversampling */
			Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
			Adafruit_BMP280::FILTER_X16,      /* Filtering */
			Adafruit_BMP280::STANDBY_MS_500); /* Standby time */
}

void barometer::baroMeter() {
	if (!barometer::bmp) {
		LOGL("Barometer not initialized!");
		return;
	}

	// Trigger a forced measurement
	if (barometer::bmp->takeForcedMeasurement()) {
		// Retrieve and log temperature
		float temp = barometer::bmp->readTemperature();
		LOG("Temperature: ");
		LOGL(" °C");

		// Retrieve and log pressure
		float press = barometer::bmp->readPressure() / 100.0f;  // Convert to hPa
		LOG("Pressure: ");
		LOGL(" hPa");

		// Retrieve and log altitude
		float altitude = barometer::bmp->readAltitude(SEALEVELPRESSURE_HPA);
		LOG("Altitude: ");
		LOGL(" m");

		// Add a delay for stability
		delay(2000);
	} else {
		LOGL("Forced measurement failed!");
	}
}

