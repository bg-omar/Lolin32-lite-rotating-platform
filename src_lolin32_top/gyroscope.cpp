//
// Created by mr on 11/17/2023.
//

#include "gyroscope.h"
#include "main.h"
#include "compass.h"

#define THRESHOLD 20

Adafruit_MPU6050 mpu; // Set the gyroscope

float   gyroscope::ax,  gyroscope::ay, gyroscope::az, gyroscope::gx, gyroscope::gy, gyroscope::gz, gyroscope::baseAx, gyroscope::baseAy, gyroscope::baseAz, gyroscope::baseGx, gyroscope::baseGy, gyroscope::baseGz, gyroscope::temperature = 0;


void gyroscope::gyroRead(){
    sensors_event_t a, gyro, temp;
    mpu.getEvent(&a, &gyro, &temp);

    gyroscope::temperature = temp.temperature;  gyroscope::ax = a.acceleration.x - gyroscope::baseAx;
    gyroscope::ay = a.acceleration.y - gyroscope::baseAy;  gyroscope::az = a.acceleration.z - gyroscope::baseAz;
    gyroscope::gx = gyro.gyro.x - gyroscope::baseGx;  gyroscope::gy = gyro.gyro.y - gyroscope::baseGy;  gyroscope::gz = gyro.gyro.z - gyroscope::baseGz;
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

    gyroscope::gyroRead();
    if(( abs(gyroscope::ax) + abs(gyroscope::ay) + abs(gyroscope::az)) > THRESHOLD){
        gyroscope::gyroFunc();
        //timers::timerTwoActive = true;      timers::timerTreeActive = true;      timers::timerButton = R1;
    }
    if(( abs(gyroscope::gx) + abs(gyroscope::gy) + abs(gyroscope::gz)) > THRESHOLD){
        gyroscope::gyroFunc();
       // timers::timerTwoActive = true;      timers::timerTreeActive = true;      timers::timerButton = L1;
    }


}
void gyroscope::gyroCalibrate_sensor() {
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
    // Try to initialize!
    if (!mpu.begin()) {
        LOGL("MPU6050 not found");
        delay(500);
		main::Found_Gyro = false;
    } else {
        LOGL("MPU6050 Found!    ");
		main::Found_Gyro = true;
        mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
        mpu.setGyroRange(MPU6050_RANGE_500_DEG);
        mpu.setFilterBandwidth(MPU6050_BAND_21_HZ); /// 5, 10, 21, 44, 94, 184, 260(off)
        gyroscope::gyroCalibrate_sensor();
        gyroscope::gyroFunc();
        delay(500);
    }
}