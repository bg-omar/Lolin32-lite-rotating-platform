//
// Created by mr on 11/17/2023.
//

#include "gyroscope.h"
#include "motor.h"
#include "main.h"

Adafruit_MPU6050 mpu; // Set the gyroscope

float   gyroscope::ax,  gyroscope::ay, gyroscope::az, gyroscope::gx, gyroscope::gy, gyroscope::gz, gyroscope::baseAx, gyroscope::baseAy, gyroscope::baseAz, gyroscope::baseGx, gyroscope::baseGy, gyroscope::baseGz, gyroscope::temperature = 0;
static int motorSequenceIndex = 0;  // Tracks current motor sequence
static int motorPower = 512;       // Default motor power (50% duty cycle)
const int maxMotorPower = 1023;    // Max PWM value
const int minMotorPower = 0;       // Min PWM value
const int totalSequences = 6;      // Number of motor sequences available


void gyroscope::gyroRead(){
    mpu.getEvent(&a, &gyro, &temp);
	delay(3);


    gyroscope::temperature = temp.temperature;
	float rawAx = a.acceleration.x - gyroscope::baseAx;
    float rawAy = a.acceleration.y - gyroscope::baseAy;
	float rawAz = a.acceleration.z - gyroscope::baseAz;
    float rawGx = gyro.gyro.x - gyroscope::baseGx;
	float rawGy = gyro.gyro.y - gyroscope::baseGy;
	float rawGz = gyro.gyro.z - gyroscope::baseGz;

	// Exponential Moving Average (EMA) filter to smooth data
	float alpha = 0.2; // Smoothing factor (0.1 - 0.3)

	// Apply smoothing filter
	gyroscope::ax = alpha * rawAx + (1 - alpha) * gyroscope::ax;
	gyroscope::ay = alpha * rawAy + (1 - alpha) * gyroscope::ay;
	gyroscope::az = alpha * rawAz + (1 - alpha) * gyroscope::az;
	gyroscope::gx = alpha * rawGx + (1 - alpha) * gyroscope::gx;
	gyroscope::gy = alpha * rawGy + (1 - alpha) * gyroscope::gy;
	gyroscope::gz = alpha * rawGz + (1 - alpha) * gyroscope::gz;
}

void gyroscope::gyroDetectMovement() {
	gyroscope::gyroRead();
// Compute dynamic threshold based on motor power
	float dynamicThreshold = THRESHOLD * (1 + (motorPower / 1023.0)); // Scale threshold with motor power


	static bool tiltDetected = false;
	static bool slantDetected = false;
	static bool verticalDetected = false;  // New flag for vertical movement

	static float accumulatedRotation = 0; // Tracks cumulative rotation
	static unsigned long lastUpdate = 0;
	static bool motorState = false;  // Tracks motor on/off state

	unsigned long currentTime = millis();
	float deltaTime = (currentTime - lastUpdate) / 1000.0; // Convert ms to seconds
	lastUpdate = currentTime;

	// Accumulate angular displacement
	accumulatedRotation += gyroscope::gz * deltaTime * 180 / PI; // Convert rad/s to degrees
\

	// Adjust motor power after 90-degree spin
	if (accumulatedRotation >= 90.0) {
		motorPower += 128; // Increase power
		motorPower = constrain(motorPower, minMotorPower, maxMotorPower);
		Serial.print("Gesture: Spin CW - Motor Power Increased to ");
		Serial.println(motorPower);
		accumulatedRotation = 0;
	} else if (accumulatedRotation <= -90.0) {
		motorPower -= 128; // Decrease power
		motorPower = constrain(motorPower, minMotorPower, maxMotorPower);
		Serial.print("Gesture: Spin CCW - Motor Power Decreased to ");
		Serial.println(motorPower);
		accumulatedRotation = 0;
	}

	// Tilt detection with dynamic threshold
	if (!tiltDetected && gyroscope::ay < -dynamicThreshold) {
		Serial.println("Gesture: Tilt Forward - Next Sequence");
		motorSequenceIndex = (motorSequenceIndex + 1) % totalSequences;
		tiltDetected = true;
	} else if (!tiltDetected && gyroscope::ay > dynamicThreshold) {
		Serial.println("Gesture: Tilt Backward - Previous Sequence");
		motorSequenceIndex = (motorSequenceIndex - 1 + totalSequences) % totalSequences;
		tiltDetected = true;
	} else if (tiltDetected && abs(gyroscope::ay) < (dynamicThreshold * 0.75)) {
		tiltDetected = false;
	}

	// Slant detection with dynamic threshold
	if (!slantDetected && gyroscope::ax < -dynamicThreshold) {
		Serial.println("Gesture: Slant Right - Next Sequence");
		motorSequenceIndex = (motorSequenceIndex + 1) % totalSequences;
		slantDetected = true;
	} else if (!slantDetected && gyroscope::ax > dynamicThreshold) {
		Serial.println("Gesture: Slant Left - Previous Sequence");
		motorSequenceIndex = (motorSequenceIndex - 1 + totalSequences) % totalSequences;
		slantDetected = true;
	} else if (slantDetected && abs(gyroscope::ax) < (dynamicThreshold * 0.75)) {
		slantDetected = false;
	}


	// 🆕 Vertical Up-Down Movement to Toggle Motor 🆕
	if (!verticalDetected && abs(gyroscope::az) > dynamicThreshold * 1.2) {  // Ensure strong enough motion
		motorState = !motorState;  // Toggle motor state
		Serial.print("Gesture: Vertical Up-Down - Motor ");
		Serial.println(motorState ? "Resumed" : "Stopped");
		verticalDetected = true;
	} else if (verticalDetected && abs(gyroscope::az) < (dynamicThreshold * 0.75)) {
		verticalDetected = false;
	}

	// 🛑 Stop motor if gesture turned it off
	if (!motorState) {
		analogWrite(motorPin, 0);  // Ensure motor is off
		return;
	}

	// Run the current motor sequence
	switch (motorSequenceIndex) {
		case 0:
			motor::rotateWithPower(motorPower);
			break;
		case 1:
			motor::pulsateWithPower(motorPower);
			break;
		case 2:
			motor::quickBurstWithPower(motorPower);
			break;
		case 3:
			motor::rampPulseWithPower(motorPower);
			break;
		case 4:
			motor::alternatingPulseWithPower(motorPower);
			break;
		case 5: // New case for random vibration sequence
			motor::randomVibrationSequence(motorPower, 3000); // Runs for 3 seconds
			break;
		case 6:
			int customPattern[] = {100, 200, 150, 250};
			motor::customSequence(customPattern, 4, motorPower);
			break;
	}
}

void gyroscope::gyroCalibrate_sensor() {
    float totX = 0;  float totY = 0;  float totZ = 0;
	float totgX = 0;  float totgY = 0;  float totgZ = 0;

	const int sampleCount = 10; // Number of samples for averaging
	unsigned long startTime = millis();

	// Collect multiple samples quickly without delay()
	for (size_t i = 0; i < sampleCount; i++) {
		mpu.getEvent(&a, &gyro, &temp);
		totX += a.acceleration.x;
		totY += a.acceleration.y;
		totZ += a.acceleration.z;
		totgX += gyro.gyro.x;
		totgY += gyro.gyro.y;
		totgZ += gyro.gyro.z;

		while (millis() - startTime < (i + 1) * 2) {
			// Wait ~2ms between each sample (faster than delay(10))
		}
	}

    gyroscope::baseAx = totX / 10;
	gyroscope::baseAy = totY / 10;
	gyroscope::baseAz = totZ / 10;
	gyroscope::baseGx = totgX / 10;
	gyroscope::baseGy = totgY / 10;
	gyroscope::baseGz = totgZ / 10;

	Serial.print(" Ax: ");  Serial.print(((baseAx)));
	Serial.print(" Ay: ");  Serial.print(((baseAy)));
	Serial.print(" Az: ");  Serial.println(((baseAz)));

	Serial.print(" Gx: ");  Serial.print(((baseGx)));
	Serial.print(" Gy: ");  Serial.print(((baseGy)));
	Serial.print(" Gz: ");  Serial.println(((baseGz)));

}

void gyroscope::gyroSetup() {
    // Try to initialize!
    if (!mpu.begin()) {
		Serial.println("MPU6050 not found");
        delay(500);

    } else {
		Serial.println("MPU6050 Found!    ");
		mpu.setAccelerometerRange(MPU6050_RANGE_2_G);   // Highest sensitivity for tilt/slant
		mpu.setGyroRange(MPU6050_RANGE_250_DEG);       // Lower noise for rotation
		mpu.setFilterBandwidth(MPU6050_BAND_10_HZ);    // Stronger noise reduction
        gyroscope::gyroCalibrate_sensor();
        gyroscope::gyroRead();
        delay(500);
    }
}