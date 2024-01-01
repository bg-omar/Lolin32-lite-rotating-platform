//
// Created by mr on 11/14/2023.
//

/*
    UNO BLE -->     DC:54:75:C3:D9:EC   -
    PC USB Dongel   00:1F:E2:C8:82:BA
    ESP 1           66:CB:3E:E9:02:8A
    ESP             3c:e9:0e:89:80:84
    ESP small cam   3C:E9:0E:88:65:16
    PS4 Controller: A4:AE:11:E1:8B:B3 (SONYWA) GooglyEyes
    PS5 Controller: 88:03:4C:B5:00:66
    */
/********************************************** Setup booting the arduino **************************************/
// section Defines
/***************************************************************************************************************/

/********************************************** Setup booting the arduino **************************************/
// section Includes
/***************************************************************************************************************/

#include <Arduino.h>
#include "esp_gap_bt_api.h"
#include <PS4Controller.h>
#include "main.h"

#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32Servo.h>
#include <driver/adc.h>



#include <SPI.h> // Not actually used but needed to compile


// ===================
// Select camera model
// ===================
//#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
//#define CAMERA_MODEL_ESP_EYE // Has PSRAM
//#define CAMERA_MODEL_ESP32S3_EYE // Has PSRAM
//#define CAMERA_MODEL_AI_THINKER // Has PSRAM
// ** Espressif Internal Boards **
#define CAMERA_MODEL_ESP32_CAM_BOARD
//#define CAMERA_MODEL_ESP32S2_CAM_BOARD
#include "camera_pins.h"







Servo myservo;  // create servo object

int posx = 100;    // variable to store the servo position


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
/********************************************** Setup booting the arduino **************************************/
// section Functions
/***************************************************************************************************************/


void onConnect() {
//    lastBattery = PS4.Battery();
    Serial.println(&"PS4 Connected!" [ PS4.Battery()]);
    Serial.println("");
}

void onDisConnect() {
    Serial.println("Disconnected!");
}

void send(int pos) {
    LOGL(pos);
    myservo.write(pos);
}

void notify() {
#if EVENTS
    boolean sqd = PS4.event.button_down.square,     squ = PS4.event.button_up.square,
                trd = PS4.event.button_down.triangle,   tru = PS4.event.button_up.triangle,
                crd = PS4.event.button_down.cross,      cru = PS4.event.button_up.cross,
                cid = PS4.event.button_down.circle,     ciu = PS4.event.button_up.circle,
                upd = PS4.event.button_down.up,         upu = PS4.event.button_up.up,
                rid = PS4.event.button_down.right,      riu = PS4.event.button_up.right,
                dod = PS4.event.button_down.down,       dou = PS4.event.button_up.down,
                led = PS4.event.button_down.left,       leu = PS4.event.button_up.left,
                l1d = PS4.event.button_down.l1,         l1u = PS4.event.button_up.l1,
                r1d = PS4.event.button_down.r1,         r1u = PS4.event.button_up.r1,
                l3d = PS4.event.button_down.l3,         l3u = PS4.event.button_up.l3,
                r3d = PS4.event.button_down.r3,         r3u = PS4.event.button_up.r3,
                psd = PS4.event.button_down.ps,         psu = PS4.event.button_up.ps,
                tpd = PS4.event.button_down.touchpad,   tpu = PS4.event.button_up.touchpad,
                opd = PS4.event.button_down.options,    opu = PS4.event.button_up.options,
                shd = PS4.event.button_down.share,      shu = PS4.event.button_up.share;

        if      (sqd) send(3110);
        else if (squ) send(3101);
        else if (crd) send(3210);
        else if (cru) send(3201);
        else if (cid) send(3310);
        else if (ciu) send(3301);
        else if (trd) send(3410);
        else if (tru) send(3401);
        else if (upd) send(1110);
        else if (upu) send(1101);
        else if (rid) send(1210);
        else if (riu) send(1201);
        else if (dod) send(1310);
        else if (dou) send(1301);
        else if (led) send(1410);
        else if (leu) send(1401);

        else if (l1d) send(2110);
        else if (l1u) send(2101);
        else if (r1d) send(2210);
        else if (r1u) send(2201);
        else if (l3d) send(2310);
        else if (l3u) send(2301);
        else if (r3d) send(2410);
        else if (r3u) send(2401);
        else if (psd) send(2510);
        else if (psu) send(2501);
        else if (tpd) send(2710);
        else if (tpu) send(2701);
        else if (shd) send(2810);
        else if (shu) send(2801);
        else if (opd) send(2910);
        else if (opu) send(2901);

#endif

#if BUTTONS
    boolean sq = PS4.Square(),
                  tr = PS4.Triangle();
          if (sq)
            Serial.print(" SQUARE pressed");
          if (tr)
            Serial.print(" TRIANGLE pressed");
          if (sq | tr)
            send();
#endif

#if JOYSTICKS
    Serial.printf("%4d, %4d, %4d, %4d, %4d, %4d \r\n",
                      (PS4.LStickX() <= -15 || PS4.LStickX() >= 15 ) ? 6127 + PS4.LStickX() : 6127,
                      (PS4.LStickY() <= -15 || PS4.LStickY() >= 15 ) ? 7127 + PS4.LStickY() : 7127,
                      (PS4.RStickX() <= -15 || PS4.RStickX() >= 15 ) ? 8127 + PS4.RStickX() : 8127,
                      (PS4.RStickY() <= -15 || PS4.RStickY() >= 15 ) ? 9127 + PS4.RStickY() : 9127,
                      (PS4.L2()) ? PS4.L2Value() : 0,
                      (PS4.R2()) ? PS4.R2Value() : 0
        );
#endif

#if SENSORS
    Serial.printf("gx:%5d,gy:%5d,gz:%5d,ax:%5d,ay:%5d,az:%5d ",
                PS4.GyrX(),
                PS4.GyrY(),
                PS4.GyrZ(),
                PS4.AccX(),
                PS4.AccY(),
                PS4.AccZ());
#endif

}








void startCameraServer();
void setupLedFlash(int pin);


/********************************************** Setup booting the arduino **************************************/
// section Setup
/***************************************************************************************************************/

void setup() {
    Serial.begin(115200);
    Serial.print("ESP32 Water Cam Rotating Version: 0.0.1 ");

    pinMode(LED_BUILTIN, OUTPUT);
    Serial.println(direction);
    Serial.setDebugOutput(true);
    Serial.begin(115200);
    PS4.attach(notify);
    PS4.attachOnConnect(onConnect);
    PS4.attachOnDisconnect(onDisConnect);
    PS4.begin("3C:E9:0E:88:65:16"); //mac Address that ESP should use

    /* Remove Paired Devices  */
    uint8_t pairedDeviceBtAddr[20][6];
    int count = esp_bt_gap_get_bond_device_num();
    esp_bt_gap_get_bond_device_list(&count, pairedDeviceBtAddr);
    for (int i = 0; i < count; i++) {
        esp_bt_gap_remove_bond_device(pairedDeviceBtAddr[i]);
    }
    Serial.println("my BT mac -> 3C:E9:0E:88:65:16");
    Serial.println("Ready for PS4");

    delay(500);

    myservo.setPeriodHertz(50);    // standard 50 hz servo
    myservo.attach(12, 1000, 2000); // attaches the servo on pin 14 to the servo object

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
#endif

#if USE_SERVER

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
    if (PS4.isConnected()) {
        if (PS4.Up()) send(posx += 1);
        if (PS4.Right()) send(100);
        if (PS4.Down()) send(posx -= 1);
        if (PS4.Left()) send(100);

//        if (PS4.UpRight())   send(1500);
//        if (PS4.DownRight()) send(1600);
//        if (PS4.DownLeft())  send(1700);
//        if (PS4.UpLeft())    send(1800);
//
//        if (PS4.L1())        send(2100);
//        if (PS4.R1())        send(2200);

//        if (PS4.L3())        send(2300);
//        if (PS4.R3())        send(2400);

//        if (PS4.PSButton())  send(2500);
//        if (PS4.Touchpad())  send(2700);
//
//        if (PS4.Share())     send(2800);
//        if (PS4.Options())   send(2900);

        if (PS4.Square()) send(100);
        if (PS4.Cross()) send(100);
        if (PS4.Circle()) send(100);
        if (PS4.Triangle()) send(100);

//        if (PS4.Charging())  send(3500);
//        if (PS4.Audio())     send(3600);
//        if (PS4.Mic())       send(3700);
//        if (PS4.Battery() < lastBattery) send(3900 + PS4.Battery());

//        if (PS4.L2()) { Serial.println(4000 + PS4.L2Value());  }
//        if (PS4.R2()) { Serial.println(5000 + PS4.R2Value());  }

        if (PS4.LStickX() <= -15 || PS4.LStickX() >= 15) { send(100 + (PS4.LStickX() / 8)); }
        if (PS4.LStickX() >= -15 || PS4.LStickX() <= 15) send(100);
        if (PS4.LStickY() <= -15 || PS4.LStickY() >= 15) { send(100 + (PS4.LStickY() / 8)); }
        if (PS4.LStickY() >= -15 || PS4.LStickY() <= 15) send(100);

//        if (PS4.LStickY() <= -15 || PS4.LStickY() >= 15 ) { Serial.println(7127 + PS4.LStickY()); } // 7 000 - 7 254
//
//        if (PS4.RStickX() <= -15 || PS4.RStickX() >= 15 ) { Serial.println(8127 + PS4.RStickX()); } // 8 000 - 8 254
//        if (PS4.RStickY() <= -15 || PS4.RStickY() >= 15 ) { Serial.println(9127 + PS4.RStickY()); } // 9 000 - 9 254

        delay(15);

    }

#if USE_GET
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
#endif
}










