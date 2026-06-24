//
// Created by mr on 11/17/2023.
//
// Gyroscope/Accelerometer (MPU6050) Sensor
// Orientation: Z-axis vertical, XY plane level/flat
// - X and Y axes are horizontal (level)
// - Z axis is vertical (pointing up/down)
// - Acceleration: az should be ~9.8 m/s² (gravity) when level
// - Gyro: gx and gy detect horizontal rotation, gz detects vertical rotation

#include "gyroscope.h"
#include "main.h"
#include "compass.h"

#define THRESHOLD 20

Adafruit_MPU6050 mpu; // Set the gyroscope

// Rotation parameters: Device is on rotating plate at 8 cm from center
const float gyroscope::ROTATION_RADIUS = 0.08f;  // 8 cm = 0.08 meters
const float gyroscope::DEVICE_MASS = 0.1f;       // Default 100g = 0.1 kg (adjust if needed)

float   gyroscope::ax = 0.0,  gyroscope::ay = 0.0, gyroscope::az = 0.0;
float   gyroscope::gx = 0.0, gyroscope::gy = 0.0, gyroscope::gz = 0.0;
float   gyroscope::baseAx = 0.0, gyroscope::baseAy = 0.0, gyroscope::baseAz = 0.0;
float   gyroscope::baseGx = 0.0, gyroscope::baseGy = 0.0, gyroscope::baseGz = 0.0;
float   gyroscope::temperature = 0.0;
float   gyroscope::rpm = 0.0;
float   gyroscope::centripetalForce = 0.0;


bool gyroscope::gyroRead(){
    sensors_event_t a, gyro, temp;
    mpu.getEvent(&a, &gyro, &temp);

    // Check if values are valid (not NaN or infinite)
    if (isnan(a.acceleration.x) || isnan(a.acceleration.y) || isnan(a.acceleration.z) ||
        isnan(gyro.gyro.x) || isnan(gyro.gyro.y) || isnan(gyro.gyro.z) ||
        isinf(a.acceleration.x) || isinf(a.acceleration.y) || isinf(a.acceleration.z) ||
        isinf(gyro.gyro.x) || isinf(gyro.gyro.y) || isinf(gyro.gyro.z)) {
        return false; // Invalid data from gyroscope
    }

    gyroscope::temperature = temp.temperature;  gyroscope::ax = a.acceleration.x - gyroscope::baseAx;
    gyroscope::ay = a.acceleration.y - gyroscope::baseAy;  gyroscope::az = a.acceleration.z - gyroscope::baseAz;
    gyroscope::gx = gyro.gyro.x - gyroscope::baseGx;  gyroscope::gy = gyro.gyro.y - gyroscope::baseGy;  gyroscope::gz = gyro.gyro.z - gyroscope::baseGz;
    return true; // Success
}

void gyroscope::gyroFunc(){
	//compass::displayCompass();
	#if LOG_VERBOSE
		(gyroscope::ax > 0)
						? LOG(" +"), LOGFloat(((gyroscope::ax)))
						: LOG(" "),  LOGFloat(((gyroscope::ax)));
		(gyroscope::ay > 0)
						? LOG(" +"), LOGFloat(((gyroscope::ay)))
						: LOG(" "),  LOGFloat(((gyroscope::ay)));
		(gyroscope::az > 0)
						? LOG(" +"), LOGFloat(((gyroscope::az)))
						: LOG(" "),  LOGFloat(((gyroscope::az)));
		(gyroscope::gx > 0)
						? LOG(" +"), LOGFloat(((gyroscope::gx)))
						: LOG(" "),  LOGFloat(((gyroscope::gx)));
		(gyroscope::gy > 0)
						? LOG(" +"), LOGFloat(((gyroscope::gy)))
						: LOG(" "),  LOGFloat(((gyroscope::gy)));
		(gyroscope::gz > 0)
						? LOG(" +"), LOGFloatln(((gyroscope::gz)))
						: LOG(" "),  LOGFloatln(((gyroscope::gz)));
	#endif
}

void gyroscope::gyroDetectMovement() {
    static unsigned long lastErrorLogMs = 0;
    static int errorCount = 0;
    unsigned long now = millis();

    if (!gyroscope::gyroRead()) {
        errorCount++;
        // Only log error every 5 seconds to avoid spam
        if ((now - lastErrorLogMs) >= 5000) {
            LOGL("Gyroscope read failed - device may not be responding");
            lastErrorLogMs = now;
            if (errorCount > 10) {
                LOGL("Gyroscope error count high - check I2C connection");
                main::Found_Gyro = false; // Mark as not found after repeated failures
            }
        }
        return;
    }
    
    // Reset error count on successful read
    if (errorCount > 0) {
        errorCount = 0;
    }

    if(( abs(gyroscope::ax) + abs(gyroscope::ay) + abs(gyroscope::az)) > THRESHOLD){
        gyroscope::gyroFunc();
        //timers::timerTwoActive = true;      timers::timerTreeActive = true;      timers::timerButton = R1;
    }
    if(( abs(gyroscope::gx) + abs(gyroscope::gy) + abs(gyroscope::gz)) > THRESHOLD){
        gyroscope::gyroFunc();
       // timers::timerTwoActive = true;      timers::timerTreeActive = true;      timers::timerButton = L1;
    }
}

float gyroscope::calculateRPM() {
    // Calculate RPM from angular velocity around Z-axis (vertical rotation)
    // RPM = (angular velocity in rad/s) * (60 seconds/minute) / (2π radians/revolution)
    // gz is angular velocity in rad/s around Z-axis
    float angularVelocityRadS = abs(gz); // Use absolute value for RPM
    rpm = (angularVelocityRadS * 60.0f) / (2.0f * PI);
    return rpm;
}

float gyroscope::calculateCentripetalForce() {
    // Calculate centripetal force: F = m * ω² * r
    // where: m = mass (kg), ω = angular velocity (rad/s), r = radius (m)
    // Using gz (angular velocity around Z-axis) for rotation on horizontal plane
    float angularVelocityRadS = abs(gz); // Use absolute value
    float omegaSquared = angularVelocityRadS * angularVelocityRadS;
    centripetalForce = DEVICE_MASS * omegaSquared * ROTATION_RADIUS;
    return centripetalForce;
}

void gyroscope::showGyro() {
    if (!gyroscope::gyroRead()) {
        LOGL("Gyroscope read failed");
        return;
    }
    
    // Calculate RPM and centripetal force
    calculateRPM();
    calculateCentripetalForce();
    
    // Display acceleration values (relative to calibration baseline)
    LOG("Accel: X=");
    LOGF(ax);
    LOG(" Y=");
    LOGF(ay);
    LOG(" Z=");
    LOGF(az);
    LOG(" m/s²  ");
    
    // Display gyro values (relative to calibration baseline)
    LOG("Gyro: X=");
    LOGF(gx);
    LOG(" Y=");
    LOGF(gy);
    LOG(" Z=");
    LOGF(gz);
    LOG(" rad/s  ");
    
    // Display RPM (revolutions per minute)
    LOG("RPM: ");
    LOGF(rpm);
    
    // Display centripetal force
    LOG("  Centripetal: ");
    LOGF(centripetalForce);
    LOGL(" N");
    
    // Also show temperature
    LOG("Temp: ");
    LOGF(temperature);
    LOGL("°C");
}
void gyroscope::gyroCalibrate_sensor() {
    // Calibrate sensor with Z-axis vertical, XY level orientation
    // When level: ax and ay should be ~0, az should be ~9.8 m/s² (gravity)
    // Gyro values should be ~0 when stationary
    float totX = 0;  float totY = 0;  float totZ = 0;  float totgX = 0;  float totgY = 0;  float totgZ = 0;
    sensors_event_t a, gyro, temp;
    delay(10);
    for (size_t i = 0; i < 10; i++) {
        mpu.getEvent(&a, &gyro, &temp);      delay(10);
        totX += a.acceleration.x;      delay(10);
        totY += a.acceleration.y;      delay(10);
        totZ += a.acceleration.z;      delay(10);
        totgX += gyro.gyro.x;      delay(10);
        totgY += gyro.gyro.y;      delay(10);
        totgZ += gyro.gyro.z;      delay(10);
    }
    gyroscope::baseAx = totX / 10;  gyroscope::baseAy = totY / 10;  gyroscope::baseAz = totZ / 10;  gyroscope::baseGx = totgX / 10;  gyroscope::baseGy = totgY / 10;  gyroscope::baseGz = totgZ / 10;
	#if LOG_VERBOSE
		LOG(" Ax: ");  LOGFloat(((baseAx)));
		LOG(" Ay: ");  LOGFloat(((baseAy)));
		LOG(" Az: ");  LOGFloatln(((baseAz)));

		LOG(" Gx: ");  LOGFloat(((baseGx)));
		LOG(" Gy: ");  LOGFloat(((baseGy)));
		LOG(" Gz: ");  LOGFloatln(((baseGz)));
	#endif
}

void gyroscope::gyroSetup() {
    if (!main::Found_I2C) return;
    // Try to initialize at common I2C addresses (0x68 is default, 0x69 is alternative)
    uint8_t addresses[] = {0x68, 0x69};
    bool found = false;
    
    for (uint8_t i = 0; i < 2; i++) {
        if (mpu.begin(addresses[i])) {
            LOG("MPU6050 Found at address 0x");
            char addrBuf[4];
            snprintf(addrBuf, sizeof(addrBuf), "%02X", addresses[i]);
            LOGL(addrBuf);
            main::Found_Gyro = true;
            found = true;
            mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
            mpu.setGyroRange(MPU6050_RANGE_500_DEG);
            mpu.setFilterBandwidth(MPU6050_BAND_21_HZ); /// 5, 10, 21, 44, 94, 184, 260(off)
            gyroscope::gyroCalibrate_sensor();
            gyroscope::gyroFunc();
            delay(500);
            break;
        }
    }
    
    if (!found) {
        LOGL("MPU6050 not found at 0x68 or 0x69");
        delay(500);
        main::Found_Gyro = false;
    }
}