
#include "main.h"
#include "steppermotor.h"
#include "barometer.h"
#include "compass.h"
#include "gyroscope.h"
#include <SPI.h> // Not actually used but needed to compile
#include "driver/adc.h"
#include "driver/ledc.h"
#include "esp_gap_bt_api.h"

#include <esp_adc_cal.h>
#include "driver/adc.h"
#include "driver/ledc.h"
#include "driver/touch_pad.h"

#include "I2Cscanner.h"
#include <PS4Controller.h>
#include <cstdint>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define OLED_SDA I2C_SDA // 23
#define OLED_SCL I2C_SCL //19

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


bool main::Found_Gyro = false;
bool main::Found_Compass = false;
bool main::Found_Barometer = false;
bool main::Found_Display = false;
bool main::Found_I2C = false;

//
int pin2channel[64]; // holds the PWM channel (0-15) attached to a given pin (0-63)

void setupAnalogWritePin(int pin, int channel, int freq=500, int resolution=8)
{
	ledcSetup(channel, freq, resolution);
	pin2channel[pin] = channel;
	ledcAttachPin(pin, pin2channel[pin]);
	ledcWrite(channel, 0);
}

void analogWriteESP32(int pin, int value)
{
	ledcWrite(pin2channel[pin], value);
}

const int SERVO = 12;         // Pin to control (pulse) servo
const int SERVO2   = 14; // Pin to middle tap of pot
const int SERVO_CHANNEL = 0;  // 0-15
const int SERVO_FREQ = 500;    // 50Hz
const int SERVO_RES = 8;     // 16 bit resolution

unsigned long lastBlinkTime = 0;
bool ledState = false;
const unsigned long blinkInterval = 1000;  // 1 second


const int MIN_PULSE = 544; // 0.48mS previously 0xFFFF/20 for 1.0mS
const int MAX_PULSE = 2400; // 2.24mS previously 0xFFFF/10 for 2.0mS

int val = 0; // For storing the reading from the POT
// the number of the LED pin
const int warmLedPin = 16;  // 16 corresponds to GPIO 16
const int coldLedPin = 17;  // 16 corresponds to GPIO 16
int coldPWM =  0; // Cold LED PWM value
int warmPWM =  0; // Warm LED PWM value

int motorSpeed = 0;
int motor2Speed =  0;
int prevSpeed, prevMs, prevDirection;
int prevMotorPWM, prevMotor2PWM, prevCold, prevWarm;

int r = 255;
int g = 0;
int b = 0;
#if PS4CONTROLLER
	float brightness = 0;
    bool forward = true;
    // Calculates the next value in a rainbow sequence
    void nextRainbowColor() {
        if (r > 0 && b == 0) { r--;  g++; }
        if (g > 0 && r == 0) { g--;  b++; }
        if (b > 0 && g == 0) { r++;  b--; }
    }

    void onConnect() {
        LOGL("PS4 Connected!");
        LOGL("");
    }

    void onDisConnect() {
        LOGL("Disconnected!");
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
        Serial.printf("gx%5dgy%5dgz%5d", PS4.GyrX(), PS4.GyrY(), PS4.GyrZ());
        Serial.printf("ax%5day%5daz%5d", PS4.AccX(), PS4.AccY(), PS4.AccZ());
    #endif
    }

#endif


void send(const char *texting) {
    LOGL((const char *) texting);
}


int direction = 1;
int speed = 0;




void setup() {
    Serial.begin(115200);
	pinMode(SERVO, PULLDOWN);
	pinMode(SERVO2, PULLDOWN);
	LOG("Servo pin");
	pinMode(led, OUTPUT);
	digitalWrite(led, LOW);
	Wire.begin(OLED_SDA, OLED_SCL);

	if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { // 0x3C is common I2C address
		Serial.println(F("SSD1306 allocation failed"));
	} else {
		main::Found_Display = true;
		LOGL("SSD1306 Display found");
		display.setRotation(2); // 0, 1, 2, or 3
		display.setTextColor(SSD1306_WHITE);
		display.setTextSize(1);
		display.setCursor(0, 0);
		display.clearDisplay();
		display.print("Initializing...");
		display.display();
		LOGL("SSD1306 Display initialized");
		delay(500);
	}

	delay(500);

	#if USE_I2C_SCANNER
		I2Cscanner::scan();
		delay(500);
	#endif

	main::printPSI_I2O();

	if (main::Found_I2C) {
		delay(500);
		gyroscope::gyroSetup();
		delay(500);

		compass::compassSetup();
		delay(500);

		barometer::baroSetup();
		delay(500);
	}

    LOGL("Stepper Motor ");
    int i;
    for(i=0;i<4;i++){
        LOG("Stepper Pin: ");
        LOGLI(motorPins[i]);
        pinMode(motorPins[i], OUTPUT);
    }

	pinMode(coldLedPin, PULLDOWN);
	pinMode(warmLedPin, PULLDOWN);


	setupAnalogWritePin(SERVO, SERVO_CHANNEL, SERVO_FREQ, SERVO_RES);
	setupAnalogWritePin(SERVO2, SERVO_CHANNEL + 1, SERVO_FREQ, SERVO_RES);


	pinMode(led, OUTPUT);
	digitalWrite(led, 0);


    #if PS4CONTROLLER
        PS4.attach(notify);
        PS4.attachOnConnect(onConnect);
        PS4.attachOnDisconnect(onDisConnect);
        PS4.begin("00:1a:7d:da:71:13"); //mac Address that ESP should use

        /* Remove Paired Devices  */
        uint8_t pairedDeviceBtAddr[20][6];
        int count = esp_bt_gap_get_bond_device_num();
        esp_bt_gap_get_bond_device_list(&count, pairedDeviceBtAddr);
        for (int i = 0; i < count; i++) {
            esp_bt_gap_remove_bond_device(pairedDeviceBtAddr[i]);
        }
        LOGL("my BT mac -> 00:1a:7d:da:71:13");
        LOGL("Ready for PS4");
    #endif
}

void loop() {

	if (main::Found_I2C) {
		if(main::Found_Gyro){
			LOGL("Ready for Found_Gyro");
			gyroscope::gyroDetectMovement();
		}

		if(main::Found_Barometer){
			LOGL("Ready for Found_Barometer");
			barometer::baroMeter();
		}

		if(main::Found_Compass){
			LOGL("Ready for Found_Compass");
			compass::showCompass();
		}
	}


#if PS4CONTROLLER
    if (PS4.isConnected() ){
		digitalWrite(led, HIGH);  // LED off when connected
		// Reset blink logic so it starts fresh next time we disconnect
		lastBlinkTime = millis();
		ledState = true;

        if (PS4.Up()) motor2Speed += 1;
        if (PS4.Right()) speed += 1;
        if (PS4.Down()) motor2Speed -= 1;
        if (PS4.Left()) speed -= 1;

        if (PS4.Square()) motor2Speed -= 50 ;
        if (PS4.Cross()) motor2Speed += 50 ;
        if (PS4.Circle()) motor2Speed = 0 ;
        if (PS4.Triangle()) motor2Speed = 200 ;

        if (PS4.L1())        motor2Speed -= 10 ;
        if (PS4.R1())        motor2Speed += 10 ;

        if (PS4.L3())        send("2300");
        //if (PS4.R3())        send("2400");
        if (PS4.Share())     send("2800");
        if (PS4.Options())   send("2900");
        if (PS4.PSButton())  send("2500");
        if (PS4.Touchpad()) {
            Serial.print("Battery Level: ");
            LOGLI(PS4.Battery());
        }

#if THUMB_STICKS
        if (PS4.LStickY() <= -15 || PS4.LStickY() >= 15 || PS4.LStickX() <= -15 || PS4.LStickX() >= 15 ) {
                Serial.printf("%4d+%4d \r\n",
                              (PS4.LStickX() <= -15 || PS4.LStickX() >= 15) ? 6127 + PS4.LStickX() : 6127,
                              (PS4.LStickY() <= -15 || PS4.LStickY() >= 15) ? 7127 + PS4.LStickY() : 7127
                );
            }

            if (PS4.RStickX() <= -15 || PS4.RStickX() >= 15 || PS4.RStickY() <= -15 || PS4.RStickY() >= 15 ) {
                Serial.printf("%4d+%4d \r\n",
                              (PS4.RStickX() <= -15 || PS4.RStickX() >= 15) ? 8127 + PS4.RStickX() : 8127,
                              (PS4.RStickY() <= -15 || PS4.RStickY() >= 15) ? 9127 + PS4.RStickY() : 9127
                );
            }
            if(PS4.L2Value() > 15 || PS4.R2Value() > 15) {
                Serial.printf("%4d+%4d \r\n",
                              (PS4.L2()) ? 4000 + PS4.L2Value() : 4000,
                              (PS4.R2()) ? 5000 + PS4.R2Value() : 5000
                );
            }
#elif BUTTONS
        if (PS4.L2()) { LOGL(4000 + PS4.L2Value());  }
            if (PS4.R2()) { LOGL(5000 + PS4.R2Value());  }
            if (PS4.LStickX() <= -15 || PS4.LStickX() >= 15 ) { LOGL(6127 + PS4.LStickX()); } // 6 000 - 6 254
            if (PS4.LStickY() <= -15 || PS4.LStickY() >= 15 ) { LOGL(7127 + PS4.LStickY()); } // 7 000 - 7 254

            if (PS4.RStickX() <= -15 || PS4.RStickX() >= 15 ) { LOGL(8127 + PS4.RStickX()); } // 8 000 - 8 254
            if (PS4.RStickY() <= -15 || PS4.RStickY() >= 15 ) { LOGL(9127 + PS4.RStickY()); } // 9 000 - 9 254
#endif

        if (PS4.Battery() < 2) {
            r = 255; g = 0;  b = 0;
            PS4.setFlashRate(25,10); // 250ms on 100ms off
        } else {
            PS4.setFlashRate(0,0); // no flashing
            nextRainbowColor();
        }

        PS4.setLed(r, g, b);
        PS4.sendToController();
		delay(25);
    } else {
		// Non-blocking LED blinking when not connected
		unsigned long currentMillis = millis();
		if (currentMillis - lastBlinkTime >= blinkInterval) {
			lastBlinkTime = currentMillis;
			ledState = !ledState; // Toggle LED state
			digitalWrite(led, ledState ? HIGH : LOW);
		}
	}
#endif
	// Ensure speed is within limits
	speed = constrain(speed, -20, 20);
    int ms = map(speed, 0, 20, 60, 3 );
    if (ms >= 62) ms = map(ms, 62, 120, 58, 3 );

    if (ms < 3 ) ms = 3;

	direction = (speed < 0) ? 0 : 1;

	if (prevSpeed != speed || prevMs != ms || prevDirection != direction) {
		LOG("speed: ");
		LOGI(speed);

		LOG("\t - ms: ");
		LOGI(ms);

		LOG("\t - direction: ");
		LOGLI(direction);
	}

    if (speed != 0) { stepperMotor::moveSteps(direction, ms, 1);
	} else {
		delay(15);
	}
	prevSpeed = speed;
	prevMs = ms;
	prevDirection = direction;

	motorSpeed = constrain(motorSpeed, 0, 200);
	motor2Speed = constrain(motor2Speed, 0, 200);
	// Clamp brightness values to valid range
	coldPWM = constrain(coldPWM, 0, 10);
	warmPWM = constrain(warmPWM, 0, 10);
	int motorPWM = map(motorSpeed, 0, 200, 0, 255);
	int motor2PWM = map(motor2Speed, 0, 200, 0, 255);
	int coldBrightness = map(coldPWM, 0, 10, 0, 255); // Negative speed dims cold LED
	int warmBrightness = map(warmPWM, 0, 10, 0, 255);  // Positive speed dims warm LED


	analogWrite(SERVO, motorPWM); // Set the motor speed using PWM
	analogWrite(SERVO2, motor2PWM); // Set the motor speed using PWM
	analogWriteESP32(coldLedPin, coldBrightness);
	analogWriteESP32(warmLedPin, warmBrightness);

	if (prevMotorPWM != motorPWM || prevMotor2PWM != motor2PWM ||
		prevCold != coldBrightness || prevWarm != warmBrightness) {
		LOG("motorSpeed: ");
		LOGI(motor2Speed);
		LOG(" \\ 200 ");
//		LOG("\t motor2Speed: ");
//		LOGI(motor2Speed);

//		LOG("\t coldBrightness ");
//		LOGI(coldBrightness);

//		LOG("\t - warmBrightness: ");
//		LOGLI(warmBrightness);
	}

	prevMotorPWM = motorPWM;
	prevMotor2PWM = motor2PWM;
	prevCold = coldBrightness;
	prevWarm = warmBrightness;
	delay(25);
}

void main::printPSI_I2O() {
	Serial.print("\t SS: ");
	Serial.print(SS);
	Serial.print("\t MOSI: ");
	Serial.print(MOSI);
	Serial.print("\t MISO: ");
	Serial.print(MISO);
	Serial.print("\t SCK: ");
	Serial.println(SCK);

	Serial.print("\t SDA: ");
	Serial.print(SDA);
	Serial.print("\t SCL: ");
	Serial.println(SCL);
	Serial.println("");
}



void main::log(const char* text) {
	Serial.print(text);
	display.print(text);
}

void main::logln(const char* text) {
	Serial.println(text);
	display.println(text);
}

void main::logDoubble(double floaty) {
	Serial.print(floaty);
	display.print(floaty);
}

void main::logDoubbleln(double floaty) {
	Serial.println(floaty);
	display.println(floaty);
}

void main::logFloat(float floaty) {
	Serial.print(floaty);
	display.print(floaty);
}

void main::logFloatln(float floaty) {
	Serial.println(floaty);
	display.println(floaty);
}

void main::logInt(int inty) {
	Serial.print(inty);
	display.print(inty);
}

void main::logIntln(int inty) {
	Serial.println(inty);
	display.println(inty);
}

void main::logHex(unsigned char hexy, int i) {
	Serial.println(hexy, i);
	display.print(hexy);
}

void main::logHexln(unsigned char hexy, int i) {
	Serial.println(hexy, i);
	display.println(hexy);
}
