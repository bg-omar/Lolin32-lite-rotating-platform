#include "main.h"
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "steppermotor.h"
#include "compass.h"
#include "gyroscope.h"
#include "aht10.h"
#include "bmp280.h"
#include "oled91.h"

#include "driver/adc.h"
#include "driver/ledc.h"
#include "esp_gap_bt_api.h"

#include <esp_adc_cal.h>
#include "driver/touch_pad.h"

#include "motor.h"
#include "I2Cscanner.h"
#include <PS4Controller.h>



Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int STICK_DEADZONE = 20;
const int MAX_STICK_RAW  = 127;   // PS4 lib returns [-127..127]
const int MAX_BOOST      = 100;

bool main::Found_Gyro = false;
bool main::Found_Compass = false;
bool main::Found_Barometer = false;
bool main::Found_AHT10 = false;
bool main::Found_Display = false;
bool main::Found_I2C = false;

bool main::motorSequence = false;
int  main::motorSequenceIndex = 0;
int  main::totalSequences = 10;

int  main::motorSpeed = 0;
int  main::motorPWM = 0;
int  main::motor2PWM = 0;
int  main::motorTimeMultiplyer = 0;
int  main::motorTimeFactor = 10;          // NEW
uint32_t main::motorSequenceEpoch = 1;    // NEW

static int motorPowerBoost = 0;
static const int maxMotorPower = 255;
static const int minMotorPower = 0;

// LEDC pin→channel map; initialize to -1 so unconfigured pins are safe
static int pin2channel[64];

static void initPin2Channel() {
  for (int i = 0; i < 64; ++i) pin2channel[i] = -1;
}

static void setupAnalogWritePin(int pin, int channel, int freq = 5000, int resolution = 8) {
  if (pin < 0 || pin >= 64) return;
  ledcSetup(channel, freq, resolution);
  pin2channel[pin] = channel;
  ledcAttachPin(pin, channel);
  ledcWrite(channel, 0);
}

// PWM config
static const int SERVO_CHANNEL = 0;   // 0..15
static const int SERVO_FREQ    = 5000;
static const int SERVO_RES     = 8;

static void initMotorOutputs() {
  initPin2Channel();

  pinMode(SERVO, OUTPUT);
  digitalWrite(SERVO, LOW);
  setupAnalogWritePin(SERVO, SERVO_CHANNEL + 0, SERVO_FREQ, SERVO_RES);
  main::analogWriteESP32(SERVO, 0);

  pinMode(SERVO2, OUTPUT);
  digitalWrite(SERVO2, LOW);
  setupAnalogWritePin(SERVO2, SERVO_CHANNEL + 1, SERVO_FREQ, SERVO_RES);
  main::analogWriteESP32(SERVO2, 0);
}

void main::analogWriteESP32(int pin, int value) {
  if (pin < 0 || pin >= 64) return;
  int ch = pin2channel[pin];
  if (ch < 0) return;                // not configured -> do nothing safely
  value = constrain(value, 0, 255);
  ledcWrite(ch, value);
}

// Motor dual pin outputs
// (pins are defined in motor.cpp; here we just configure LEDC channels)

// LEDs (warmLedPin=16 and coldLedPin=17 do NOT conflict with I2C pins 4/5)
static const int warmLedPin = 16;
static const int coldLedPin = 17;

static bool g_ledPwmEnabled = false;

static unsigned long lastBlinkTime = 0;
static bool ledState = false;
static const unsigned long blinkInterval = 1000;

static int prevSpeed = 0, prevMs = 0, prevDirection = 0;
static int prevMotorPWM = -1, prevMotor2PWM = -1, prevCold = -1, prevWarm = -1;

static unsigned long lastSeqChangeMs = 0;
static const unsigned long SEQ_CHANGE_COOLDOWN_MS = 250;

static int r = 255, g = 0, b = 0;

#if PS4CONTROLLER
static void nextRainbowColor() {
  if (r > 0 && b == 0) { r--; g++; }
  if (g > 0 && r == 0) { g--; b++; }
  if (b > 0 && g == 0) { r++; b--; }
}

static void onConnect() {
  LOGL("PS4 Connected!");
  LOGL("");
}

static void onDisConnect() {
  LOGL("Disconnected!");
}

// PS4 event callback
static void notify() {
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

  if      (sqd) main::motorSpeed = 100;
  else if (squ) LOGI(3101);
  else if (crd) main::motorSpeed = 50;
  else if (cru) LOGI(3201);
  else if (cid) main::motorSpeed = 0;
  else if (ciu) LOGI(3301);
  else if (trd) main::motorSpeed = 150;
  else if (tru) LOGI(3401);
  else if (upd) main::motorSpeed += 1;
  else if (upu) LOGI(1101);
  else if (rid) LOGLI(1210);
  else if (riu) LOGI(1201);
  else if (dod) main::motorSpeed -= 1;
  else if (dou) LOGI(1301);
  else if (led) LOGLI(1410);
  else if (leu) LOGI(1401);
  else if (l1d) main::motorSpeed -= 10;
  else if (l1u) LOGI(2101);
  else if (r1d) main::motorSpeed += 10;
  else if (r1u) LOGI(2201);
  else if (l3d) LOGLI(2310);
  else if (l3u) LOGI(2301);
  else if (r3d) LOGLI(2410);
  else if (r3u) LOGI(2401);
  else if (psd) LOGLI(2510);
  else if (psu) LOGI(2501);
  else if (tpd) LOGLI(2710);
  else if (tpu) LOGI(2701);
  else if (shd) LOGLI(2810);
  else if (shu) LOGI(2801);
  else if (opd) {
    main::motorSequence = !main::motorSequence;
    main::motorSequenceEpoch++; // NEW: reset sequence statics on toggle
    if (main::motorSequence) LOGL("Motor sequence started");
    else                     LOGL("Motor sequence stopped");
  }
  else if (opu) {
    LOGI(2901);
  }
#endif
}
#endif // PS4CONTROLLER

void send(const char *texting) {
  LOGL(texting);
}

static int direction = 1;
static int speed = 0;

void setup() {
  // Motor OFF first — before Serial, I2C scan, or sensor delays
  initMotorOutputs();

  Serial.begin(115200);
  
  // Configure brown-out detector to prevent resets from voltage drops
  // Set BOD threshold to 2.5V (default is 2.43V, which is too sensitive for motors)
  // This helps prevent resets when motor draws current and causes voltage drops
  #ifdef ESP32
    // ESP32 brown-out detector configuration
    // Note: Some ESP32 variants may not support all BOD levels
    // 0 = Disable (not recommended)
    // 1 = 2.43V (default, very sensitive)
    // 2 = 2.50V (recommended for motor applications)
    // 3 = 2.67V (more tolerant)
    // 4 = 2.80V (very tolerant, may miss real brown-outs)
    // We'll use 2.5V which is a good balance
    // Note: This may need to be set via menuconfig or sdkconfig in PlatformIO
    // For now, we'll add a note and use watchdog instead
  #endif
  
  // Disable watchdog for setup (will enable later if needed)
  // ESP32 has task watchdog that can cause resets if loop() takes too long
  // We'll feed it in loop() to prevent resets during motor operation


  main::scan_PSI_I2C();

  // Initialize each sensor independently - each handles its own failure gracefully
  // This allows the script to continue even if some I2C devices are not connected
  delay(500);
  oled91::oled91Setup();
  delay(500);
  gyroscope::gyroSetup();
  delay(500);
  compass::compassSetup();
  delay(500);
  aht10::aht10Setup();
  delay(500);
  bmp280::bmp280Setup();
  delay(500);

  LOG("Servo pin: "); LOGLI(SERVO);
  LOG("Servo pin 2: "); LOGLI(SERVO2);

  // Status LED
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);

  // Warm/cold LED PWM: protect against I2C conflict (warmLedPin==I2C_SDA in your config)
  bool ledPinsConflict =
      (warmLedPin == I2C_SDA) || (warmLedPin == I2C_SCL) ||
      (coldLedPin == I2C_SDA) || (coldLedPin == I2C_SCL);

  if (ledPinsConflict) {
    LOGL("WARN: warm/cold LED pins conflict with I2C pins; LED PWM disabled to keep I2C stable.");
    g_ledPwmEnabled = false;
  } else {
    pinMode(coldLedPin, OUTPUT);
    pinMode(warmLedPin, OUTPUT);
    setupAnalogWritePin(coldLedPin, SERVO_CHANNEL + 2, SERVO_FREQ, SERVO_RES);
    setupAnalogWritePin(warmLedPin, SERVO_CHANNEL + 3, SERVO_FREQ, SERVO_RES);
    main::analogWriteESP32(coldLedPin, 0);
    main::analogWriteESP32(warmLedPin, 0);
    g_ledPwmEnabled = true;
  }

  // Stepper setup (unchanged)
  LOGL("Stepper Motor ");
  for (int i = 0; i < 4; i++) {
    LOG("Stepper Pin: ");
    LOGLI(motorPins[i]);
    pinMode(motorPins[i], OUTPUT);
  }

  // OLED init still optional; logging is guarded by Found_Display
  // if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { ... }

#if PS4CONTROLLER
  PS4.attach(notify);
  PS4.attachOnConnect(onConnect);
  PS4.attachOnDisconnect(onDisConnect);
  PS4.begin("00:1a:7d:da:71:13");

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

static unsigned long lastCompassMs = 0;
static unsigned long lastAHT10Ms = 0;
static unsigned long lastBMP280Ms = 0;
static unsigned long lastGyroShowMs = 0;
static unsigned long lastGyroCheckMs = 0;
static unsigned long lastOLED91Ms = 0;
static bool gyroReadyLogged = false;

void loop() {
    unsigned long now = millis();
    
    // Feed watchdog to prevent resets during long operations
    // ESP32 has task watchdog that resets if loop() takes too long
    #ifdef ESP32
      yield();  // Feed watchdog and allow other tasks to run
    #endif
    
    // Each sensor function checks its own Found_X flag internally
    // This allows the script to continue even if I2C devices are not connected
    if (main::Found_Compass && (now - lastCompassMs) >= 500) {
        compass::showCompass();
        lastCompassMs = now;
    }

    if (main::Found_AHT10 && (now - lastAHT10Ms) >= 2000) {
        aht10::showAHT10();
        lastAHT10Ms = now;
    }

    if (main::Found_Barometer && (now - lastBMP280Ms) >= 2000) {
        bmp280::showBMP280();
        lastBMP280Ms = now;
    }

    if (main::Found_Gyro) {
      // Only log "Ready" message once, or every 5 seconds if needed for debugging
      if (!gyroReadyLogged || (now - lastGyroCheckMs) >= 5000) {
        if (!gyroReadyLogged) {
          LOGL("Gyroscope ready");
          gyroReadyLogged = true;
        }
        lastGyroCheckMs = now;
      }
      
      // Show gyroscope values periodically (every 1 second)
      // This will show acceleration differences from calibration (tilt/orientation changes)
      if ((now - lastGyroShowMs) >= 1000) {
        gyroscope::showGyro();
        lastGyroShowMs = now;
      }
      
      // Also detect movement (exceeds threshold)
      gyroscope::gyroDetectMovement();
    }

    // Update OLED display with sensor data (every 500ms for smooth updates)
    if (main::Found_Display && (now - lastOLED91Ms) >= 500) {
        oled91::showSensorData();
        lastOLED91Ms = now;
    }

#if PS4CONTROLLER
  if (PS4.isConnected()) {
    digitalWrite(led, LOW);
    lastBlinkTime = millis();
    ledState = true;

    if (PS4.PSButton()) send("2500");
    if (PS4.Touchpad()) {
      Serial.print("Battery Level: ");
      LOGLI(PS4.Battery());
    }

    unsigned long now = millis();

    // L2/R2: sequence index change (rate-limited) + epoch bump to reset statics
    if (PS4.L2Value() > 15 && (now - lastSeqChangeMs) > SEQ_CHANGE_COOLDOWN_MS) {
      lastSeqChangeMs = now;
      main::motorSequenceIndex = (main::motorSequenceIndex - 1) % main::totalSequences;
      if (main::motorSequenceIndex < 0) main::motorSequenceIndex += main::totalSequences;
      main::motorSequenceEpoch++;
      LOG("Motor sequence index: "); LOGI(main::motorSequenceIndex);
      LOG(" - Total sequences: "); LOGLI(main::totalSequences);
    }

    if (PS4.R2Value() > 15 && (now - lastSeqChangeMs) > SEQ_CHANGE_COOLDOWN_MS) {
      lastSeqChangeMs = now;
      main::motorSequenceIndex = (main::motorSequenceIndex + 1) % main::totalSequences;
      main::motorSequenceEpoch++;
      LOG("Motor sequence index: "); LOGI(main::motorSequenceIndex);
      LOG(" - Total sequences: "); LOGLI(main::totalSequences);
    }

#if THUMB_STICKS
    int lxy = PS4.LStickY();
    int lxx = PS4.LStickX();
    int rxy = PS4.RStickY();
    int rxx = PS4.RStickX();

    // BOOST from Y
    int rawBoost = 0;
    if (abs(lxy) > STICK_DEADZONE) rawBoost = lxy;
    else if (abs(rxy) > STICK_DEADZONE) rawBoost = rxy;

    long mappedBoost = map(rawBoost, -MAX_STICK_RAW, MAX_STICK_RAW, -MAX_BOOST, MAX_BOOST);
    motorPowerBoost = (int)constrain(mappedBoost, -MAX_BOOST, MAX_BOOST);

    // TIME MULTIPLIER from X
    int rawMul = 0;
    if (abs(lxx) > STICK_DEADZONE) rawMul = lxx;
    else if (abs(rxx) > STICK_DEADZONE) rawMul = rxx;

    main::motorTimeMultiplyer = (int)constrain(map(rawMul, -MAX_STICK_RAW, MAX_STICK_RAW, -10, 10), -10, 10);

    // NEW: clamped factor used everywhere in motor.cpp
    main::motorTimeFactor = constrain(10 + main::motorTimeMultiplyer, 1, 20);
#endif

    if (PS4.Battery() < 2) {
      r = 255; g = 0; b = 0;
      PS4.setFlashRate(25, 10);
    } else {
      PS4.setFlashRate(0, 0);
      nextRainbowColor();
    }

    PS4.setLed(r, g, b);
    PS4.sendToController();
    delay(25);
  } else {
    unsigned long currentMillis = millis();
    if (currentMillis - lastBlinkTime >= blinkInterval) {
      lastBlinkTime = currentMillis;
      ledState = !ledState;
      digitalWrite(led, ledState ? HIGH : LOW);
    }
  }
#endif

  // Stepper section unchanged
  speed = constrain(speed, -20, 20);
  int ms = map(speed, 0, 20, 60, 3);
  if (ms >= 62) ms = map(ms, 62, 120, 58, 3);
  if (ms < 3) ms = 3;

  direction = (speed < 0) ? 0 : 1;

  if (prevSpeed != speed || prevMs != ms || prevDirection != direction) {
    LOG("speed: "); LOGI(speed);
    LOG("\t - ms: "); LOGI(ms);
    LOG("\t - direction: "); LOGLI(direction);
  }

  if (speed != 0) stepperMotor::moveSteps(direction, ms, 1);
  else delay(15);

  prevSpeed = speed;
  prevMs = ms;
  prevDirection = direction;

  // Motor PWM computation (preserve your logic, just clamp correctly)
  int motorSpeedPWM = constrain(main::motorSpeed + motorPowerBoost, 0, 250);

  // Keep your map but prevent overflow >255
  int targetPWM = map(motorSpeedPWM, 0, 200, 0, 255);
  targetPWM = constrain(targetPWM, 0, 255);
  
  // Soft-start/ramp to prevent sudden current spikes that cause voltage drops
  // Gradually change PWM instead of instant changes to reduce reset risk
  static int lastTargetPWM = 0;
  static unsigned long lastPWMChangeMs = 0;
  const int PWM_RAMP_RATE = 5;  // Max change per 10ms (adjust for smoother/slower)
  const unsigned long PWM_RAMP_INTERVAL = 10;  // 10ms between ramp steps
  
  unsigned long nowPWM = millis();
  if (nowPWM - lastPWMChangeMs >= PWM_RAMP_INTERVAL) {
    if (targetPWM > main::motorPWM) {
      main::motorPWM = min(main::motorPWM + PWM_RAMP_RATE, targetPWM);
    } else if (targetPWM < main::motorPWM) {
      main::motorPWM = max(main::motorPWM - PWM_RAMP_RATE, targetPWM);
    }
    lastPWMChangeMs = nowPWM;
  }
  
  // If target changed significantly, update immediately (for safety/stopping)
  if (abs(targetPWM - lastTargetPWM) > 50) {
    main::motorPWM = targetPWM;  // Allow fast changes for large differences
  }
  lastTargetPWM = targetPWM;

  // LEDs (only if enabled and configured)
  int coldPWM = 0, warmPWM = 0;
  coldPWM = constrain(coldPWM, 0, 10);
  warmPWM = constrain(warmPWM, 0, 10);

  int coldBrightness = map(coldPWM, 0, 10, 0, 255);
  int warmBrightness = map(warmPWM, 0, 10, 0, 255);

  // Run motor pattern if enabled; otherwise write steady PWM to both pins
  motor::sequence();
  if (!main::motorSequence) {
    motor::write((uint8_t)main::motorPWM);
  }

  if (g_ledPwmEnabled) {
    main::analogWriteESP32(coldLedPin, coldBrightness);
    main::analogWriteESP32(warmLedPin, warmBrightness);
  }

  if (prevMotorPWM != main::motorPWM || prevMotor2PWM != main::motor2PWM ||
      prevCold != coldBrightness || prevWarm != warmBrightness) {
    LOG("motorPWM: "); LOGI(main::motorPWM); LOG(" \\ 255 ");
    LOG("\tBoost: "); LOGI(motorPowerBoost);
    LOG("\tTimeMult: "); LOGLI(main::motorTimeMultiplyer);
  }

  prevMotorPWM = main::motorPWM;
  prevMotor2PWM = main::motor2PWM;
  prevCold = coldBrightness;
  prevWarm = warmBrightness;

  delay(50);
}

void main::scan_PSI_I2C() {
  Serial.print("\t SS: "); Serial.print(SS);
  Serial.print("\t MOSI: "); Serial.print(MOSI);
  Serial.print("\t MISO: "); Serial.print(MISO);
  Serial.print("\t SCK: "); Serial.println(SCK);

  Serial.print("\t SDA: "); Serial.print(SDA);
  Serial.print("\t SCL: "); Serial.println(SCL);
  Serial.println("");

#if USE_I2C_SCANNER
  I2Cscanner::scan();
  delay(500);
#endif
}

// ---- Logging (now safe if OLED not initialized) ----
void main::log(const char* text) {
  Serial.print(text);
  if (Found_Display) display.print(text);
}
void main::logln(const char* text) {
  Serial.println(text);
  if (Found_Display) display.println(text);
}
void main::logInt(int inty) {
  Serial.print(inty);
  if (Found_Display) display.print(inty);
}
void main::logIntln(int inty) {
  Serial.println(inty);
  if (Found_Display) display.println(inty);
}
void main::logDoubble(double floaty) {
  Serial.print(floaty);
  if (Found_Display) display.print(floaty);
}
void main::logDoubbleln(double floaty) {
  Serial.println(floaty);
  if (Found_Display) display.println(floaty);
}
void main::logFloat(float floaty) {
  Serial.print(floaty);
  if (Found_Display) display.print(floaty);
}
void main::logFloatln(float floaty) {
  Serial.println(floaty);
  if (Found_Display) display.println(floaty);
}
void main::logHex(unsigned char hexy, int i) {
  Serial.println(hexy, i);
  if (Found_Display) display.print(hexy);
}
void main::logHexln(unsigned char hexy, int i) {
  Serial.println(hexy, i);
  if (Found_Display) display.println(hexy);
}