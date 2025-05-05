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

    delay(200);
    for(address = 1; address < 127; address++ ) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0) {
			LOG(" 0x");
            if (address<16) {
				LOGL("0");
            }
#if LOG_DEBUG
            if (main::Found_Display) LOGHexln(address, HEX);
#endif
			LOG(reinterpret_cast<const char *>(address));
			LOG(", ");
            nDevices++;
            delay(200);
        }
        else if (error==4) {
			LOG("Unknown error at address 0x");
            if (address<16) {
				LOG("0 ");
            }
#if LOG_DEBUG
            if (main::Found_Display) LOGHexln(address, HEX);
#endif
			LOG(reinterpret_cast<const char *>(address));
        }
    }
    delay(20);
	LOG(" devices: ");
    LOGL(reinterpret_cast<const char *>(nDevices));

    delay(100);
    if (nDevices == 0) {
        LOGL("-- No I2C devices found--");
    }
}