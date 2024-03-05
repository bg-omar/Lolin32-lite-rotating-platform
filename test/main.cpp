//
// Created by mr on 11/14/2023.
//

#include "main.h"
//#include <RH_ASK.h>
#include <Arduino.h>
#include "main.h"


#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32Servo.h>
#include <driver/adc.h>
#include "../.pio/libdeps/esp32cam/RadioHead/RH_ASK.h"

#ifdef RH_HAVE_HARDWARE_SPI
#include <SPI.h> // Not actually used but needed to compile
#endif


RH_ASK driver(2000, 4, 5, 0); // ESP8266 or ESP32: do not use pin 11 or 2
// RH_ASK driver(2000, 3, 4, 0); // ATTiny, RX on D3 (pin 2 on attiny85) TX on D4 (pin 3 on attiny85),
// RH_ASK driver(2000, PD14, PD13, 0); STM32F4 Discovery: see tx and rx on Orange and Red LEDS





#define USE_GET 1
Servo myservo;  // create servo object

int posx, output14 = 0;    // variable to store the servo position


// ===========================
// Enter your WiFi credentials
// ===========================
const char* ssid = "KT2";
const char* password = "PestoP3s10";


//Your Domain name with URL path or IP address with path
String serverName = "http://192.168.1.8:1880/update-sensor";

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
unsigned long timerDelay = 5000;

void startCameraServer();
void setupLedFlash(int pin);

String httpGETRequest(const char* serverName) {
    HTTPClient http;

    // Your IP address with path or Domain name with URL path
    http.begin(serverName);

    // If you need Node-RED/server authentication, insert user and password below
    //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");


    // Send HTTP POST request
    int httpResponseCode = http.GET();

    String payload = "{}";

    if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        payload = http.getString();
    }
    else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();

    return payload;
}


void setup() {
    Serial.begin(115200);
    Serial.println("Version: ");
    LOGL("Serial started");
    pinMode(LED_BUILTIN, OUTPUT);
    LOG(direction);

    Serial.setDebugOutput(true);
    Serial.println();



    myservo.setPeriodHertz(50);    // standard 50 hz servo
    myservo.attach(12, 1000, 2000); // attaches the servo on pin 14 to the servo object
    pinMode(14,INPUT);
    pinMode(15,INPUT);
    adcAttachPin(14);
    adcAttachPin(15);


#if USE_GET
    WiFi.begin(ssid, password);
    Serial.println("Connecting");
    while(WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());

    Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
#else

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.frame_size = FRAMESIZE_UXGA;
    config.pixel_format = PIXFORMAT_JPEG;  // for streaming
    //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.jpeg_quality = 12;
    config.fb_count = 1;

    // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
    //                      for larger pre-allocated frame buffer.
    if (config.pixel_format == PIXFORMAT_JPEG) {
        if (psramFound()) {
            config.jpeg_quality = 10;
            config.fb_count = 2;
            config.grab_mode = CAMERA_GRAB_LATEST;
        } else {
            // Limit the frame size when PSRAM is not available
            config.frame_size = FRAMESIZE_SVGA;
            config.fb_location = CAMERA_FB_IN_DRAM;
        }
    } else {
        // Best option for face detection/recognition
        config.frame_size = FRAMESIZE_240X240;
#if CONFIG_IDF_TARGET_ESP32S3
        config.fb_count = 2;
#endif
    }

#if defined(CAMERA_MODEL_ESP_EYE)
    pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }

    sensor_t* s = esp_camera_sensor_get();
    // initial sensors are flipped vertically and colors are a bit saturated
    if (s->id.PID == OV3660_PID) {
        s->set_vflip(s, 1);        // flip it back
        s->set_brightness(s, 1);   // up the brightness just a bit
        s->set_saturation(s, -2);  // lower the saturation
    }
    // drop down frame size for higher initial frame rate
    if (config.pixel_format == PIXFORMAT_JPEG) {
        s->set_framesize(s, FRAMESIZE_QVGA);
    }

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
    s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

#if defined(CAMERA_MODEL_ESP32S3_EYE)
    s->set_vflip(s, 1);
#endif

// Setup LED FLash if LED pin is defined in camera_pins.h
#if defined(LED_GPIO_NUM)
    setupLedFlash(LED_GPIO_NUM);
#endif

    WiFi.begin(ssid, password);
    WiFi.setSleep(false);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");

    startCameraServer();

    Serial.print("Camera Ready! Use 'http://");
    Serial.print(WiFi.localIP());
    Serial.println("' to connect");
#endif

}

void loop() {


    uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
    uint8_t buflen = sizeof(buf);

    if (driver.recv(buf, &buflen)) // Non-blocking
    {
        int i;

        // Message with a good checksum received, dump it.
        driver.printBuffer("Got:", buf, buflen);
    }

    posx += 1;
    myservo.write(posx);

    const char *serverNm = "http://192.168.1.8:1880/update-sensor";
    const String &sensorReadings = httpGETRequest((const char *) serverNm);

    if ((millis() - lastTime) > timerDelay) {
        //Check WiFi connection status
        if(WiFi.status()== WL_CONNECTED){
            HTTPClient http;

            String serverPath = serverName + "?temperature=24.37";

            // Your Domain name with URL path or IP address with path
            http.begin(serverPath.c_str());

            // If you need Node-RED/server authentication, insert user and password below
            //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");

            // Send HTTP GET request
            int httpResponseCode = http.GET();

            if (httpResponseCode>0) {
                Serial.print("HTTP Response code: ");
                Serial.println(httpResponseCode);
                String payload = http.getString();
                Serial.println(payload);
            }
            else {
                Serial.print("Error code: ");
                Serial.println(httpResponseCode);
            }
            // Free resources
            http.end();
        }
        else {
            Serial.println("WiFi Disconnected");
        }
        lastTime = millis();
    }
}










