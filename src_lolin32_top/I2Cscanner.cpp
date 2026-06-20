//
// Created by mr on 11/20/2023.
//


#include "main.h"
#include "I2Cscanner.h"
#include <Wire.h>

void I2Cscanner::scan() {
    byte error, address;
    int nDevices;

    LOGL("I2C Scanning...");
    nDevices = 0;

    // Try to scan I2C bus - continue even if scan fails
    delay(200);
    for(address = 1; address < 127; address++ ) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0) {
			LOG(" 0x");
            if (address<16) {
				LOG("0");
            }
#if LOG_DEBUG
            if (main::Found_Display) LOGHexln(address, HEX);
#endif
            // Print address as hex, NOT as pointer
            char buf[6];
            snprintf(buf, sizeof(buf), "%02X", address);
            LOG(buf);
			LOG(", ");
            nDevices++;
            delay(200);
        }
        else if (error==4) {
			LOG("Unknown error at address 0x");
            if (address<16) {
				LOG("0");
            }
#if LOG_DEBUG
            if (main::Found_Display) LOGHexln(address, HEX);
#endif
            char buf[6];
            snprintf(buf, sizeof(buf), "%02X", address);
            LOG(buf);
        }
        // Other errors (1, 2, 3, 5) are normal - device not present or busy
    }
    delay(20);
	LOG(" devices: ");

    // Print nDevices as integer, not as pointer
    char numBuf[8];
    snprintf(numBuf, sizeof(numBuf), "%d", nDevices);
    LOGL(numBuf);

    delay(100);
    if (nDevices == 0) {
        LOGL("-- No I2C devices found--");
        LOGL("-- Continuing anyway - sensors will initialize independently --");
		main::Found_I2C = false;
	} else {
		LOGL("-- I2C scan completed --");
		main::Found_I2C = true;
    }
}