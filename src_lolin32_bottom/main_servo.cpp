#include <Arduino.h>

// ===================== User pins =====================
#define LED_BUILTIN 22
#define POTENTIOMETER 34   // ADC input-only

// L298N (Channel A)
static const uint8_t L298_ENA = 25;   // PWM (remove ENA jumper on the board)
static const uint8_t L298_IN1 = 26;
static const uint8_t L298_IN2 = 27;

// Optional encoder (comment out if unused)
static const uint8_t ENC_A = 32;      // Yellow
static const uint8_t ENC_B = 33;      // Green
volatile int32_t encCount = 0;

// ===================== PWM (LEDC) =====================
static const uint8_t  PWM_CH   = 0;
static const uint32_t PWM_FREQ = 20000; // 20kHz
static const uint8_t  PWM_RES  = 8;     // 0..255

// ===================== Tuning =====================
static const float DEAD_BAND = 0.05f; // 5% center deadband
static const int   RAMP_STEP = 10;    // PWM step per update tick
static const uint32_t CONTROL_DT_MS = 20;

// Pot calibration window: sweep to min and max once after boot
static const uint32_t CAL_TIME_MS = 2500;

// ===================== Helpers =====================
static inline float clampf(float x, float a, float b) { return (x < a) ? a : (x > b) ? b : x; }

static inline int rampTo(int cur, int tgt, int step) {
  if (cur < tgt) return min(cur + step, tgt);
  if (cur > tgt) return max(cur - step, tgt);
  return cur;
}

void send(const String &texting) {
  Serial.println(texting);
}

// Motor driver: signed PWM in [-255..255]
void motorSetSignedPWM(int pwmSigned) {
  pwmSigned = constrain(pwmSigned, -255, 255);

  if (pwmSigned > 0) {
    digitalWrite(L298_IN1, HIGH);
    digitalWrite(L298_IN2, LOW);
    ledcWrite(PWM_CH, (uint8_t)pwmSigned);
  } else if (pwmSigned < 0) {
    digitalWrite(L298_IN1, LOW);
    digitalWrite(L298_IN2, HIGH);
    ledcWrite(PWM_CH, (uint8_t)(-pwmSigned));
  } else {
    // Coast (less heating). For brake: set IN1=HIGH IN2=HIGH
    digitalWrite(L298_IN1, LOW);
    digitalWrite(L298_IN2, LOW);
    ledcWrite(PWM_CH, 0);
  }
}

// Optional encoder ISR
void IRAM_ATTR isrEncA() {
  bool a = digitalRead(ENC_A);
  bool b = digitalRead(ENC_B);
  encCount += (a == b) ? +1 : -1; // flip sign if direction is inverted
}

// ===================== Pot calibration state =====================
static bool     calDone = false;
static uint32_t tBoot   = 0;
static int      potMin  = 4095;
static int      potMax  = 0;

// Motor state
static int motorTarget  = 0;  // [-255..255]
static int motorApplied = 0;  // ramped

// Map pot ADC -> throttle [-1..+1] using calibrated min/max and piecewise center
float potToThrottle(int potRaw) {
  // Safety: if not calibrated well, default neutral
  if (potMax - potMin < 50) return 0.0f;

  int center = (potMin + potMax) / 2;

  int leftSpan  = center - potMin;
  int rightSpan = potMax - center;

  if (leftSpan < 10)  leftSpan  = 10;
  if (rightSpan < 10) rightSpan = 10;

  float dx = (float)potRaw - (float)center;
  float x;

  if (dx < 0) x = dx / (float)leftSpan;   // hits -1 at potMin
  else        x = dx / (float)rightSpan;  // hits +1 at potMax

  x = clampf(x, -1.0f, +1.0f);

  // Deadband
  if (fabsf(x) < DEAD_BAND) x = 0.0f;

  return x;
}

void setup() {
  Serial.begin(115200);
  delay(200);

  send("ESP32 Lolin32 Lite: Pot -> DC motor (-100%..+100%) via L298N");
  send("Startup calibration: within ~2.5s sweep pot to MIN and MAX once.");

  pinMode(LED_BUILTIN, OUTPUT);

  // GPIO34: input-only, no internal pullups
  pinMode(POTENTIOMETER, INPUT);

  // L298N
  pinMode(L298_IN1, OUTPUT);
  pinMode(L298_IN2, OUTPUT);

  ledcSetup(PWM_CH, PWM_FREQ, PWM_RES);
  ledcAttachPin(L298_ENA, PWM_CH);
  ledcWrite(PWM_CH, 0);

  // Optional encoder
  pinMode(ENC_A, INPUT_PULLUP);
  pinMode(ENC_B, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENC_A), isrEncA, CHANGE);

  motorSetSignedPWM(0);
  tBoot = millis();
}

void loop() {
  const uint32_t now = millis();

  // ---------- Read pot ----------
  int potRaw = analogRead(POTENTIOMETER); // 0..4095

  // ---------- Calibrate pot min/max during startup window ----------
  if (!calDone) {
    potMin = min(potMin, potRaw);
    potMax = max(potMax, potRaw);

    if (now - tBoot > CAL_TIME_MS) {
      calDone = true;
      Serial.print("Calibration done. potMin="); Serial.print(potMin);
      Serial.print(" potMax="); Serial.println(potMax);
      if (potMax - potMin < 300) {
        send("Warning: pot range is small. Check wiring (pot ends should be 3V3 and GND).");
      }
    }
  } else {
    // Optional: keep expanding min/max if user goes beyond initial sweep
    potMin = min(potMin, potRaw);
    potMax = max(potMax, potRaw);
  }

  // ---------- Compute throttle and target PWM ----------
  float throttle = potToThrottle(potRaw);        // [-1..+1]
  motorTarget = (int)lroundf(throttle * 255.0f); // [-255..255]

  // ---------- Apply ramp + output ----------
  static uint32_t tCtrl = 0;
  if (now - tCtrl >= CONTROL_DT_MS) {
    motorApplied = rampTo(motorApplied, motorTarget, RAMP_STEP);
    motorSetSignedPWM(motorApplied);
    tCtrl = now;
  }

  // ---------- Debug prints (similar to your old script) ----------
  static uint32_t tPrint = 0;
  if (now - tPrint >= 100) {
    int32_t c;
    noInterrupts(); c = encCount; interrupts();

    Serial.print("pot=");
    Serial.print(potRaw);
    Serial.print("\tthr=");
    Serial.print(throttle * 100.0f, 1);
    Serial.print("%\tpwm=");
    Serial.print(motorApplied);
    Serial.print("\tmin=");
    Serial.print(potMin);
    Serial.print("\tmax=");
    Serial.print(potMax);
    Serial.print("\tcount=");
    Serial.println(c);

    tPrint = now;
  }

  delay(10);
}