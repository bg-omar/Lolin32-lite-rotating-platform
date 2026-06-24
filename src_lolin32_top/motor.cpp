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

#ifndef MOTOR_TEST_LOG
#define MOTOR_TEST_LOG 1
#endif

static void logMotorTestHeaderOnce() {
#if MOTOR_TEST_LOG
  static bool printed = false;
  if (!printed) {
    Serial.println("MOTOR_TEST_HEADER,ms,epoch,seq_index,test,event,phase,elapsed_ms,pwm,base_pwm,pulse_pwm");
    printed = true;
  }
#endif
}

static void logMotorTestEvent(
  const char* testName,
  const char* eventName,
  const char* phaseName,
  unsigned long elapsedMs,
  int pwm,
  int basePwm,
  int pulsePwm
) {
#if MOTOR_TEST_LOG
  logMotorTestHeaderOnce();

  Serial.print("MOTOR_TEST,");
  Serial.print(millis());
  Serial.print(",");
  Serial.print(main::motorSequenceEpoch);
  Serial.print(",");
  Serial.print(main::motorSequenceIndex);
  Serial.print(",");
  Serial.print(testName);
  Serial.print(",");
  Serial.print(eventName);
  Serial.print(",");
  Serial.print(phaseName);
  Serial.print(",");
  Serial.print(elapsedMs);
  Serial.print(",");
  Serial.print(pwm);
  Serial.print(",");
  Serial.print(basePwm);
  Serial.print(",");
  Serial.println(pulsePwm);
#endif
}

static int testBasePwm() {
  // If controller base PWM is 0, use a conservative fallback.
  // For 10-bit PWM this is ~204; for 8-bit this is ~51.
  int base = main::motorPWM;
  if (base <= 0) base = MOTOR_PWM_MAX / 5;
  return constrain(base, 0, MOTOR_PWM_MAX);
}

static int testPulseUpPwm(int basePwm, int deltaPwm) {
  return constrain(basePwm + deltaPwm, 0, MOTOR_PWM_MAX);
}

static int testPulseDownPwm(int basePwm, int deltaPwm) {
  return constrain(basePwm - deltaPwm, 0, MOTOR_PWM_MAX);
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

      // Hydrodynamic memory / vorticity-column tests
      case 10: motor::controlPulseNoPreSpin(); break;
      case 11: motor::preconditionPulseTest10s(); break;
      case 12: motor::preconditionPulseTest30s(); break;
      case 13: motor::preconditionPulseTest60s(); break;
      case 14: motor::preconditionPulseTest120s(); break;
      case 15: motor::steadyRotorOnlyTest60s(); break;
      case 16: motor::memoryDecayAfterStopTest(); break;
      case 17: motor::negativePulseAfterPreconditionTest(); break;

      default:
        motor::write(0);
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
    motor::write(0);
  }

  const int tf = main::motorTimeFactor; // clamped [1..20]
  if (millis() - sequenceStart >= (unsigned long)durationMs * (unsigned long)tf) {
    motor::write(0);
    return;
  }

  unsigned long randomInterval = (unsigned long)random(50 * tf, 300 * tf);

  if (millis() - lastUpdate >= randomInterval) {
    lastUpdate = millis();

    if (isOn) {
      motor::write(0);
      isOn = false;
    } else {
      currentPower = random(main::motorPWM / 2, main::motorPWM);
      currentPower = constrain(currentPower, 0, MOTOR_PWM_MAX);
      motor::write(currentPower);
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
    motor::write(0);
  }

  const int tf = main::motorTimeFactor;
  unsigned long dt = (unsigned long)(isOn ? random(50 * tf, 150 * tf) : 0);

  if (millis() - lastUpdate >= dt) {
    lastUpdate = millis();
    isOn = !isOn;
    motor::write(isOn ? main::motorPWM : 0);
  }
}

void motor::longPulse() {
  static uint32_t lastEpoch = 0;
  static unsigned long lastUpdate = 0;
  static bool isOn = false;

  if (epochChanged(lastEpoch)) {
    lastUpdate = millis();
    isOn = false;
    motor::write(0);
  }

  const int tf = main::motorTimeFactor;
  unsigned long dt = (unsigned long)(isOn ? random(300 * tf, 600 * tf) : 0);

  if (millis() - lastUpdate >= dt) {
    lastUpdate = millis();
    isOn = !isOn;
    motor::write(isOn ? main::motorPWM : 0);
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
    motor::write(0);
  }

  const int tf = main::motorTimeFactor;
  if (totalPulses <= 0) totalPulses = random(3 * tf, 6 * tf);

  if (millis() - lastUpdate >= (unsigned long)random(50 * tf, 150 * tf)) {
    lastUpdate = millis();

    if (pulseCount < totalPulses) {
      int duty = highPower ? main::motorPWM : (main::motorPWM / 2);
      motor::write(duty);
      highPower = !highPower;
      if (!highPower) pulseCount++;
    } else {
      motor::write(0);
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
    motor::write(0);
  }

  if (size <= 0) return;

  if (millis() - lastUpdate >= (unsigned long)pattern[index]) {
    lastUpdate = millis();
    motor::write(((index % 2) == 0) ? main::motorPWM : 0);
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
    motor::write(0);
  }

  const int tf = main::motorTimeFactor;

  if (millis() - lastUpdate >= (unsigned long)(20 * tf)) {
    lastUpdate = millis();

    if (increasing) {
      speed += 20;
      if (speed >= main::motorPWM) increasing = false;
    } else {
      speed -= 20;
      if (speed <= 0) increasing = true;
    }
    speed = constrain(speed, 0, MOTOR_PWM_MAX);
    motor::write(speed);
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
    motor::write(0);
  }

  const int tf = main::motorTimeFactor;
  unsigned long dt = (unsigned long)((isOn ? 200 : 100) * tf);

  if (millis() - lastUpdate >= dt) {
    lastUpdate = millis();
    isOn = !isOn;

    if (isOn) {
      motor::write(main::motorPWM);
      pulseCount++;
    } else {
      motor::write(0);
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
    motor::write(0);
  }

  const int tf = main::motorTimeFactor;
  unsigned long dt = (unsigned long)((state == 0 ? 300 : 100) * tf);

  if (millis() - lastUpdate >= dt) {
    lastUpdate = millis();
    if (state == 0) {
      motor::write(main::motorPWM);
      state = 1;
    } else {
      motor::write(0);
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
    motor::write(0);
  }

  const int tf = main::motorTimeFactor;

  if (millis() - lastUpdate >= (unsigned long)(10 * tf)) {
    lastUpdate = millis();

    if (state == 0) {
      speed += 20;
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
      speed -= 20;
      if (speed <= 0) {
        speed = 0;
        state = 0;
      }
    }

    speed = constrain(speed, 0, MOTOR_PWM_MAX);
    motor::write(speed);
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
    motor::write(0);
  }

  const int tf = main::motorTimeFactor;

  if (millis() - lastUpdate >= (unsigned long)(150 * tf)) {
    lastUpdate = millis();

    if (pulseCount < 5) {
      int duty = highPower ? main::motorPWM : (main::motorPWM / 2);
      motor::write(duty);
      highPower = !highPower;
      if (!highPower) pulseCount++;
    } else {
      motor::write(0);
      pulseCount = 0;
    }
  }
}

void motor::runPreconditionThenPulse(
  const char* testName,
  unsigned long preMs,
  unsigned long pulseMs,
  unsigned long postMs,
  int deltaPwm,
  bool negativePulse
) {
  static uint32_t lastEpoch = 0;
  static unsigned long startMs = 0;
  static unsigned long lastBeatMs = 0;
  static int phase = -1;
  static int basePwm = 0;
  static int pulsePwm = 0;

  if (epochChanged(lastEpoch)) {
    startMs = millis();
    lastBeatMs = 0;
    phase = -1;

    basePwm = testBasePwm();
    pulsePwm = negativePulse
      ? testPulseDownPwm(basePwm, deltaPwm)
      : testPulseUpPwm(basePwm, deltaPwm);

    motor::write(0);
    logMotorTestEvent(testName, "RESET", "INIT", 0, 0, basePwm, pulsePwm);
  }

  unsigned long now = millis();
  unsigned long elapsed = now - startMs;

  int newPhase;
  const char* phaseName;
  int pwm;

  if (elapsed < preMs) {
    newPhase = 0;
    phaseName = "PRECONDITION_BASE_ROTATION";
    pwm = basePwm;
  } else if (elapsed < preMs + pulseMs) {
    newPhase = 1;
    phaseName = negativePulse ? "NEGATIVE_PULSE" : "POSITIVE_PULSE";
    pwm = pulsePwm;
  } else if (elapsed < preMs + pulseMs + postMs) {
    newPhase = 2;
    phaseName = "POST_BASE_ROTATION";
    pwm = basePwm;
  } else {
    newPhase = 3;
    phaseName = "DONE_HOLD_BASE";
    pwm = basePwm;
  }

  if (newPhase != phase) {
    phase = newPhase;
    logMotorTestEvent(testName, "PHASE", phaseName, elapsed, pwm, basePwm, pulsePwm);
  }

  if (now - lastBeatMs >= 1000) {
    lastBeatMs = now;
    logMotorTestEvent(testName, "BEAT", phaseName, elapsed, pwm, basePwm, pulsePwm);
  }

  motor::write(pwm);
}

void motor::runNoPreconditionPulseControl(
  const char* testName,
  unsigned long idleMs,
  unsigned long pulseMs,
  unsigned long postMs,
  int deltaPwm
) {
  static uint32_t lastEpoch = 0;
  static unsigned long startMs = 0;
  static unsigned long lastBeatMs = 0;
  static int phase = -1;
  static int basePwm = 0;
  static int pulsePwm = 0;

  if (epochChanged(lastEpoch)) {
    startMs = millis();
    lastBeatMs = 0;
    phase = -1;

    basePwm = testBasePwm();
    pulsePwm = testPulseUpPwm(basePwm, deltaPwm);

    motor::write(0);
    logMotorTestEvent(testName, "RESET", "INIT", 0, 0, basePwm, pulsePwm);
  }

  unsigned long now = millis();
  unsigned long elapsed = now - startMs;

  int newPhase;
  const char* phaseName;
  int pwm;

  if (elapsed < idleMs) {
    newPhase = 0;
    phaseName = "NO_PRESPIN_IDLE";
    pwm = 0;
  } else if (elapsed < idleMs + pulseMs) {
    newPhase = 1;
    phaseName = "PULSE_WITHOUT_PRECONDITION";
    pwm = pulsePwm;
  } else if (elapsed < idleMs + pulseMs + postMs) {
    newPhase = 2;
    phaseName = "POST_OFF";
    pwm = 0;
  } else {
    newPhase = 3;
    phaseName = "DONE_OFF";
    pwm = 0;
  }

  if (newPhase != phase) {
    phase = newPhase;
    logMotorTestEvent(testName, "PHASE", phaseName, elapsed, pwm, basePwm, pulsePwm);
  }

  if (now - lastBeatMs >= 1000) {
    lastBeatMs = now;
    logMotorTestEvent(testName, "BEAT", phaseName, elapsed, pwm, basePwm, pulsePwm);
  }

  motor::write(pwm);
}

void motor::runSteadyOnlyTest(
  const char* testName,
  unsigned long holdMs
) {
  static uint32_t lastEpoch = 0;
  static unsigned long startMs = 0;
  static unsigned long lastBeatMs = 0;
  static int phase = -1;
  static int basePwm = 0;

  if (epochChanged(lastEpoch)) {
    startMs = millis();
    lastBeatMs = 0;
    phase = -1;
    basePwm = testBasePwm();

    motor::write(0);
    logMotorTestEvent(testName, "RESET", "INIT", 0, 0, basePwm, basePwm);
  }

  unsigned long now = millis();
  unsigned long elapsed = now - startMs;

  int newPhase;
  const char* phaseName;
  int pwm;

  if (elapsed < holdMs) {
    newPhase = 0;
    phaseName = "STEADY_BASE_ONLY_NO_PULSE";
    pwm = basePwm;
  } else {
    newPhase = 1;
    phaseName = "DONE_HOLD_BASE";
    pwm = basePwm;
  }

  if (newPhase != phase) {
    phase = newPhase;
    logMotorTestEvent(testName, "PHASE", phaseName, elapsed, pwm, basePwm, basePwm);
  }

  if (now - lastBeatMs >= 1000) {
    lastBeatMs = now;
    logMotorTestEvent(testName, "BEAT", phaseName, elapsed, pwm, basePwm, basePwm);
  }

  motor::write(pwm);
}

void motor::runMemoryDecayAfterStop(
  const char* testName,
  unsigned long preMs,
  unsigned long stopMs,
  unsigned long pulseMs,
  unsigned long postMs,
  int deltaPwm
) {
  static uint32_t lastEpoch = 0;
  static unsigned long startMs = 0;
  static unsigned long lastBeatMs = 0;
  static int phase = -1;
  static int basePwm = 0;
  static int pulsePwm = 0;

  if (epochChanged(lastEpoch)) {
    startMs = millis();
    lastBeatMs = 0;
    phase = -1;

    basePwm = testBasePwm();
    pulsePwm = testPulseUpPwm(basePwm, deltaPwm);

    motor::write(0);
    logMotorTestEvent(testName, "RESET", "INIT", 0, 0, basePwm, pulsePwm);
  }

  unsigned long now = millis();
  unsigned long elapsed = now - startMs;

  int newPhase;
  const char* phaseName;
  int pwm;

  if (elapsed < preMs) {
    newPhase = 0;
    phaseName = "PRECONDITION_BASE_ROTATION";
    pwm = basePwm;
  } else if (elapsed < preMs + stopMs) {
    newPhase = 1;
    phaseName = "STOP_DECAY_WINDOW";
    pwm = 0;
  } else if (elapsed < preMs + stopMs + pulseMs) {
    newPhase = 2;
    phaseName = "PULSE_AFTER_STOP";
    pwm = pulsePwm;
  } else if (elapsed < preMs + stopMs + pulseMs + postMs) {
    newPhase = 3;
    phaseName = "POST_BASE_ROTATION";
    pwm = basePwm;
  } else {
    newPhase = 4;
    phaseName = "DONE_HOLD_BASE";
    pwm = basePwm;
  }

  if (newPhase != phase) {
    phase = newPhase;
    logMotorTestEvent(testName, "PHASE", phaseName, elapsed, pwm, basePwm, pulsePwm);
  }

  if (now - lastBeatMs >= 1000) {
    lastBeatMs = now;
    logMotorTestEvent(testName, "BEAT", phaseName, elapsed, pwm, basePwm, pulsePwm);
  }

  motor::write(pwm);
}

void motor::controlPulseNoPreSpin() {
    // Direct-pumping control.
    // If this produces the same wave as preconditioned tests, then direct pumping is not excluded.
    runNoPreconditionPulseControl(
      "CONTROL_NO_PRESPIN",
      3000,   // idle before pulse
      300,    // pulse duration
      8000,   // observe after pulse
      MOTOR_PWM_MAX / 8
    );
}

void motor::preconditionPulseTest10s() {
    runPreconditionThenPulse(
      "PRE_10S_THEN_PULSE",
      10000,
      300,
      10000,
      MOTOR_PWM_MAX / 8,
      false
    );
}

void motor::preconditionPulseTest30s() {
    runPreconditionThenPulse(
      "PRE_30S_THEN_PULSE",
      30000,
      300,
      10000,
      MOTOR_PWM_MAX / 8,
      false
    );
}

void motor::preconditionPulseTest60s() {
    runPreconditionThenPulse(
      "PRE_60S_THEN_PULSE",
      60000,
      300,
      10000,
      MOTOR_PWM_MAX / 8,
      false
    );
}

void motor::preconditionPulseTest120s() {
    runPreconditionThenPulse(
      "PRE_120S_THEN_PULSE",
      120000,
      300,
      10000,
      MOTOR_PWM_MAX / 8,
      false
    );
}

void motor::steadyRotorOnlyTest60s() {
    // False-positive control.
    // There should be no upward pulse because no pulse is applied.
    runSteadyOnlyTest(
      "STEADY_ONLY_60S_NO_PULSE",
      60000
    );
}

void motor::memoryDecayAfterStopTest() {
    // Builds the column, stops rotor briefly, then pulses.
    // If effect weakens after stop, the carrier has finite memory/decay.
    runMemoryDecayAfterStop(
      "MEMORY_DECAY_AFTER_STOP",
      60000,  // precondition
      10000,  // stop/decay window
      300,    // pulse
      10000,  // observe after pulse
      MOTOR_PWM_MAX / 8
    );
}

void motor::negativePulseAfterPreconditionTest() {
    // Negative torsion control.
    // Should weaken/reverse the surface-pull response compared to positive pulse.
    runPreconditionThenPulse(
      "NEGATIVE_PULSE_AFTER_PRE_60S",
      60000,
      300,
      10000,
      MOTOR_PWM_MAX / 10,
      true
    );
}