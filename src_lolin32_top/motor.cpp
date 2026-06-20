#include "motor.h"

// Define motor pins here (single definition)
int motor::motorPinA = SERVO;
int motor::motorPinB = SERVO2;

// Helper: reset-on-epoch-change
static inline bool epochChanged(uint32_t &lastEpoch) {
  if (lastEpoch != main::motorSequenceEpoch) {
    lastEpoch = main::motorSequenceEpoch;
    return true;
  }
  return false;
}

void motor::sequence() {
  if (main::motorSequence) {
    if (main::motorSequenceIndex < 0 || main::motorSequenceIndex >= main::totalSequences) {
      main::motorSequenceIndex = 0;
      main::motorSequenceEpoch++;
    }

    switch (main::motorSequenceIndex) {
      case 0: motor::rotateWithPower(); break;
      case 1: motor::pulsateWithPower(); break;
      case 2: motor::quickBurstWithPower(); break;
      case 3: motor::rampPulseWithPower(); break;
      case 4: motor::alternatingPulseWithPower(); break;
      case 5: motor::randomVibrationSequence(3000); break;
      case 6: motor::shortPulse(); break;
      case 7: motor::alternatingPulse(); break;
      case 8: motor::longPulse(); break;
      case 9: {
        int customPattern[] = {100, 200, 150, 250};
        motor::customSequence(customPattern, 4);
        break;
      }
      default:
        motor::write((uint8_t)0);
        break;
    }
  }
}

void motor::randomVibrationSequence(int durationMs) {
  static uint32_t lastEpoch = 0;
  static unsigned long sequenceStart = 0;
  static unsigned long lastUpdate = 0;
  static bool isOn = false;
  static int currentPower = 0;

  if (epochChanged(lastEpoch)) {
    sequenceStart = millis();
    lastUpdate = millis();
    isOn = false;
    currentPower = 0;
    motor::write((uint8_t)0);
  }

  const int tf = main::motorTimeFactor; // clamped [1..20]
  if (millis() - sequenceStart >= (unsigned long)durationMs * (unsigned long)tf) {
    motor::write((uint8_t)0);
    return;
  }

  unsigned long randomInterval = (unsigned long)random(50 * tf, 300 * tf);

  if (millis() - lastUpdate >= randomInterval) {
    lastUpdate = millis();

    if (isOn) {
      motor::write((uint8_t)0);
      isOn = false;
    } else {
      currentPower = random(main::motorPWM / 2, main::motorPWM);
      currentPower = constrain(currentPower, 0, 255);
      motor::write((uint8_t)currentPower);
      isOn = true;

      int pattern = random(0, 3);
      switch (pattern) {
        case 0: shortPulse(); break;
        case 1: longPulse(); break;
        case 2: alternatingPulse(); break;
      }
    }
  }
}

void motor::shortPulse() {
  static uint32_t lastEpoch = 0;
  static unsigned long lastUpdate = 0;
  static bool isOn = false;

  if (epochChanged(lastEpoch)) {
    lastUpdate = millis();
    isOn = false;
    motor::write((uint8_t)0);
  }

  const int tf = main::motorTimeFactor;
  unsigned long dt = (unsigned long)(isOn ? random(50 * tf, 150 * tf) : 0);

  if (millis() - lastUpdate >= dt) {
    lastUpdate = millis();
    isOn = !isOn;
    motor::write((uint8_t)(isOn ? main::motorPWM : 0));
  }
}

void motor::longPulse() {
  static uint32_t lastEpoch = 0;
  static unsigned long lastUpdate = 0;
  static bool isOn = false;

  if (epochChanged(lastEpoch)) {
    lastUpdate = millis();
    isOn = false;
    motor::write((uint8_t)0);
  }

  const int tf = main::motorTimeFactor;
  unsigned long dt = (unsigned long)(isOn ? random(300 * tf, 600 * tf) : 0);

  if (millis() - lastUpdate >= dt) {
    lastUpdate = millis();
    isOn = !isOn;
    motor::write((uint8_t)(isOn ? main::motorPWM : 0));
  }
}

void motor::alternatingPulse() {
  static uint32_t lastEpoch = 0;
  static unsigned long lastUpdate = 0;
  static bool highPower = true;
  static int pulseCount = 0;
  static int totalPulses = 0;

  if (epochChanged(lastEpoch)) {
    lastUpdate = millis();
    highPower = true;
    pulseCount = 0;
    totalPulses = 0;
    motor::write((uint8_t)0);
  }

  const int tf = main::motorTimeFactor;
  if (totalPulses <= 0) totalPulses = random(3 * tf, 6 * tf);

  if (millis() - lastUpdate >= (unsigned long)random(50 * tf, 150 * tf)) {
    lastUpdate = millis();

    if (pulseCount < totalPulses) {
      uint8_t duty = (uint8_t)(highPower ? main::motorPWM : (main::motorPWM / 2));
      motor::write(duty);
      highPower = !highPower;
      if (!highPower) pulseCount++;
    } else {
      motor::write((uint8_t)0);
      pulseCount = 0;
      totalPulses = random(3 * tf, 6 * tf);
    }
  }
}

void motor::customSequence(int pattern[], int size) {
  static uint32_t lastEpoch = 0;
  static int index = 0;
  static unsigned long lastUpdate = 0;

  if (epochChanged(lastEpoch)) {
    index = 0;
    lastUpdate = millis();
    motor::write((uint8_t)0);
  }

  if (size <= 0) return;

  if (millis() - lastUpdate >= (unsigned long)pattern[index]) {
    lastUpdate = millis();
    motor::write((uint8_t)(((index % 2) == 0) ? main::motorPWM : 0));
    index = (index + 1) % size;
  }
}

void motor::rotateWithPower() {
  static uint32_t lastEpoch = 0;
  static unsigned long lastUpdate = 0;
  static int speed = 0;
  static bool increasing = true;

  if (epochChanged(lastEpoch)) {
    lastUpdate = millis();
    speed = 0;
    increasing = true;
    motor::write((uint8_t)0);
  }

  const int tf = main::motorTimeFactor;

  if (millis() - lastUpdate >= (unsigned long)(20 * tf)) {
    lastUpdate = millis();

    if (increasing) {
      speed += 5;
      if (speed >= main::motorPWM) increasing = false;
    } else {
      speed -= 5;
      if (speed <= 0) increasing = true;
    }
    speed = constrain(speed, 0, 255);
    motor::write((uint8_t)speed);
  }
}

void motor::pulsateWithPower() {
  static uint32_t lastEpoch = 0;
  static unsigned long lastUpdate = 0;
  static bool isOn = false;
  static int pulseCount = 0;

  if (epochChanged(lastEpoch)) {
    lastUpdate = millis();
    isOn = false;
    pulseCount = 0;
    motor::write((uint8_t)0);
  }

  const int tf = main::motorTimeFactor;
  unsigned long dt = (unsigned long)((isOn ? 200 : 100) * tf);

  if (millis() - lastUpdate >= dt) {
    lastUpdate = millis();
    isOn = !isOn;

    if (isOn) {
      motor::write((uint8_t)main::motorPWM);
      pulseCount++;
    } else {
      motor::write((uint8_t)0);
    }

    if (pulseCount >= 5) pulseCount = 0;
  }
}

void motor::quickBurstWithPower() {
  static uint32_t lastEpoch = 0;
  static unsigned long lastUpdate = 0;
  static int state = 0;

  if (epochChanged(lastEpoch)) {
    lastUpdate = millis();
    state = 0;
    motor::write((uint8_t)0);
  }

  const int tf = main::motorTimeFactor;
  unsigned long dt = (unsigned long)((state == 0 ? 300 : 100) * tf);

  if (millis() - lastUpdate >= dt) {
    lastUpdate = millis();
    if (state == 0) {
      motor::write((uint8_t)main::motorPWM);
      state = 1;
    } else {
      motor::write((uint8_t)0);
      state = 0;
    }
  }
}

void motor::rampPulseWithPower() {
  static uint32_t lastEpoch = 0;
  static unsigned long lastUpdate = 0;
  static int speed = 0;
  static int state = 0;
  static unsigned long holdStart = 0;

  if (epochChanged(lastEpoch)) {
    lastUpdate = millis();
    speed = 0;
    state = 0;
    holdStart = 0;
    motor::write((uint8_t)0);
  }

  const int tf = main::motorTimeFactor;

  if (millis() - lastUpdate >= (unsigned long)(10 * tf)) {
    lastUpdate = millis();

    if (state == 0) {
      speed += 5;
      if (speed >= main::motorPWM) {
        speed = main::motorPWM;
        state = 1;
        holdStart = millis();
      }
    } else if (state == 1) {
      if (millis() - holdStart >= (unsigned long)(200 * tf)) {
        state = 2;
      }
    } else if (state == 2) {
      speed -= 5;
      if (speed <= 0) {
        speed = 0;
        state = 0;
      }
    }

    speed = constrain(speed, 0, 255);
    motor::write((uint8_t)speed);
  }
}

void motor::alternatingPulseWithPower() {
  static uint32_t lastEpoch = 0;
  static unsigned long lastUpdate = 0;
  static bool highPower = true;
  static int pulseCount = 0;

  if (epochChanged(lastEpoch)) {
    lastUpdate = millis();
    highPower = true;
    pulseCount = 0;
    motor::write((uint8_t)0);
  }

  const int tf = main::motorTimeFactor;

  if (millis() - lastUpdate >= (unsigned long)(150 * tf)) {
    lastUpdate = millis();

    if (pulseCount < 5) {
      uint8_t duty = (uint8_t)(highPower ? main::motorPWM : (main::motorPWM / 2));
      motor::write(duty);
      highPower = !highPower;
      if (!highPower) pulseCount++;
    } else {
      motor::write((uint8_t)0);
      pulseCount = 0;
    }
  }
}