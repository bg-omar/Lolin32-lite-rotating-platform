//
// Created by mr on 12/6/2023.
//
#include <Arduino.h>
#include "decodeJson.h"

JSONVar myObject = JSON.parse(sensorReadings);

void decodeJson::DecodeJson () {
// JSON.typeof(jsonVar) can be used to get the type of the var
    if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Parsing input failed!");
        return;
    }

    Serial.print("JSON object = ");
    Serial.println(myObject);

// myObject.keys() can be used to get an array of all the keys in the object
    JSONVar keys = myObject.keys();

    for (int i = 0; i < keys.length(); i++) {
        JSONVar value = myObject[keys[i]];
        Serial.print(keys[i]);
        Serial.print(" = ");
        Serial.println(value);
        sensorReadingsArr[i] = double(value);
    }
    Serial.print("1 = ");
    Serial.println(sensorReadingsArr[0]);
    Serial.print("2 = ");
    Serial.println(sensorReadingsArr[1]);
    Serial.print("3 = ");
    Serial.println(sensorReadingsArr[2]);
}