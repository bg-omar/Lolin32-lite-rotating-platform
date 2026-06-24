//
// Created by mr on 11/20/2023.
//


#include "main.h"
#include "I2Cscanner.h"
#include <Wire.h>

void I2Cscanner::scan() {
    Wire.begin(I2C_SDA, I2C_SCL);
    delay(50);

    byte error, address;
    int nDevices = 0;

    LOGL("I2C scanning...");

    for (address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0) {
            LOG("  Found 0x");
            if (address < 16) LOG("0");
            char buf[6];
            snprintf(buf, sizeof(buf), "%02X", address);
            LOGL(buf);
            nDevices++;
        }
        // error 1–5: no device at this address — normal, don't log
    }

    if (nDevices == 0) {
        LOGL("No I2C devices — sensor init skipped");
        main::Found_I2C = false;
    } else {
        LOG("I2C scan done, ");
        LOGI(nDevices);
        LOGL(" device(s)");
        main::Found_I2C = true;
    }
}