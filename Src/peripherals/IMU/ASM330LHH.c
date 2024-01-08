/*
 * ASM330LHH.c  (IMU interface)
 *
 *	- Jan  6, 2024
 *		Author		 : Darsh
 *		Log			 : Edited function names to follow consistent naming
 *
 *  - Apr 29, 2023 (Creation)
 *      Author       : Tim S.
 *      Contributors : nithinsenthil , Raphael
 *      Log          : IMU Control functions written
 */

#include "ASM330LHH.h"

/*************************** IMU Helper Functions *************************/

/**
 * Configures the accelerometer control register of an IMU device.
 *
 * @param acel_rate The rate of the accelerometer.
 * @param acel_scale The scale of the accelerometer.
 * @param digital_filter_on Whether the digital filter is enabled or not.
 *
 * @returns None
 */
void imu_acelCtrl(int acel_rate, int acel_scale, int digital_filter_on) {
	acel_rate &= 0xFF;
	acel_scale &= 0xF;
	digital_filter_on &= 1;
	int data = acel_rate << 4 | acel_scale << 2 | digital_filter_on << 1;
	softi2c_writeReg(IMU_I2C, IMU_ADDR, 0x10, data);
}

/**
 * Configures the gyroscope rate and scale for the IMU sensor.
 *
 * @param gyro_rate The rate of the gyroscope.
 * @param gyro_scale The scale of the gyroscope.
 *
 * @returns None
 */
void imu_gyroCtrl(int gyro_rate, int gyro_scale) {
	gyro_rate &= 0xFF;
	gyro_scale &= 0xFF;
	int data = gyro_rate << 4 | gyro_scale;
	softi2c_writeReg(IMU_I2C, IMU_ADDR, 0x11, data);
}

/*************************** IMU Interface Functions *************************/

void imu_init(int acel_rate, int acel_scale, int gyro_rate, int gyro_scale) {
	softi2c_init(IMU_I2C);

	softi2c_writeReg(IMU_I2C, IMU_ADDR, 0x12, 0x01); // soft reset imu
	nop(100);
	imu_acelCtrl(acel_rate, acel_scale, 0);
	imu_gyroCtrl(gyro_rate, gyro_scale);
}

int16_t imu_readAcel_X() {
	return softi2c_readRegHighLow(IMU_I2C, IMU_ADDR, 0x29, 0x28);
}

int16_t imu_readAcel_Y() {
	return softi2c_readRegHighLow(IMU_I2C, IMU_ADDR, 0x2B, 0x2A);
}

int16_t imu_readAcel_Z() {
	return softi2c_readRegHighLow(IMU_I2C, IMU_ADDR, 0x2D, 0x2C);
}

int16_t imu_readGyro_X() {
	return softi2c_readRegHighLow(IMU_I2C, IMU_ADDR, 0x23, 0x22);
}

int16_t imu_readGyro_Y() {
	return softi2c_readRegHighLow(IMU_I2C, IMU_ADDR, 0x25, 0x24);
}

int16_t imu_readGyro_Z() {
	return softi2c_readRegHighLow(IMU_I2C, IMU_ADDR, 0x27, 0x26);
}

int16_t imu_readTemp() {
	return softi2c_readRegHighLow(IMU_I2C, IMU_ADDR, 0x21, 0x20);
}
