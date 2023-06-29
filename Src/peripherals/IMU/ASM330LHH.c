/*
 * ASM330LHH.c  (IMU interface)
 *
 *  - Apr 29, 2023 (Creation)
 *      Author       : Tim S.
 *      Contributors : nithinsenthil , Raphael
 *      Log          : IMU Control functions written
 */

#include "ASM330LHH.h"

/**
 * Configures the accelerometer control register of an IMU device.
 *
 * @param acel_rate The rate of the accelerometer.
 * @param acel_scale The scale of the accelerometer.
 * @param digital_filter_on Whether the digital filter is enabled or not.
 *
 * @returns None
 */
void op1_imu_acel_ctrl(int acel_rate, int acel_scale, int digital_filter_on) {
	acel_rate &= 0xFF;
	acel_scale &= 0xF;
	digital_filter_on &= 1;
	int data = acel_rate << 4 | acel_scale << 2 | digital_filter_on << 1;
	softi2c_write_reg(OP1_I2C2, IMU_ADDR, 0x10, data);
}

/**
 * Configures the gyroscope rate and scale for the IMU sensor.
 *
 * @param gyro_rate The rate of the gyroscope.
 * @param gyro_scale The scale of the gyroscope.
 *
 * @returns None
 */
void op1_imu_gyro_ctrl(int gyro_rate, int gyro_scale) {
	gyro_rate &= 0xFF;
	gyro_scale &= 0xFF;
	int data = gyro_rate << 4 | gyro_scale;
	softi2c_write_reg(OP1_I2C2, IMU_ADDR, 0x11, data);
}

/**
 * Initializes the OP1 IMU with the given accelerometer and gyroscope settings.
 *
 * @param acel_rate The accelerometer data rate.
 * @param acel_scale The accelerometer full scale range.
 * @param gyro_rate The gyroscope data rate.
 * @param gyro_scale The gyroscope full scale range.
 *
 * @returns None
 */
void op1_imu_init(int acel_rate, int acel_scale, int gyro_rate, int gyro_scale) {
	softi2c_write_reg(OP1_I2C2, IMU_ADDR, 0x12, 0x01); // soft reset imu
	nop(100);
	op1_imu_acel_ctrl(acel_rate, acel_scale, 0);
	op1_imu_gyro_ctrl(gyro_rate, gyro_scale);
}

/**
 * Reads the X-axis acceleration value from the IMU sensor connected to the I2C2 bus of OP1.
 *
 * @returns The X-axis acceleration value as a 16-bit signed integer.
 */
int16_t op1_imu_read_acel_x() {
	return softi2c_read_reg_hl(OP1_I2C2, IMU_ADDR, 0x29, 0x28);
}

/**
 * Reads the y-axis acceleration value from the IMU sensor connected to the I2C2 bus of OP1.
 *
 * @returns The y-axis acceleration value as a 16-bit signed integer.
 */
int16_t op1_imu_read_acel_y() {
	return softi2c_read_reg_hl(OP1_I2C2, IMU_ADDR, 0x2B, 0x2A);
}

/**
 * Reads the acceleration value of the z-axis from the IMU sensor connected to the I2C2 bus of OP1.
 *
 * @returns The acceleration value of the z-axis.
 */
int16_t op1_imu_read_acel_z() {
	return softi2c_read_reg_hl(OP1_I2C2, IMU_ADDR, 0x2D, 0x2C);
}

/**
 * Reads the X-axis gyroscope data from the IMU sensor connected to the I2C2 bus of OP1.
 *
 * @returns The X-axis gyroscope data as a 16-bit signed integer.
 */
int16_t op1_imu_read_gyro_x() {
	return softi2c_read_reg_hl(OP1_I2C2, IMU_ADDR, 0x23, 0x22);
}

/**
 * Reads the y-axis gyroscope value from the IMU sensor connected to the I2C2 bus of OP1.
 *
 * @returns The y-axis gyroscope value as a 16-bit signed integer.
 */
int16_t op1_imu_read_gyro_y() {
	return softi2c_read_reg_hl(OP1_I2C2, IMU_ADDR, 0x25, 0x24);
}

/**
 * Reads the Z-axis gyroscope data from the IMU sensor connected to the I2C2 bus of OP1.
 *
 * @returns The Z-axis gyroscope data as a 16-bit signed integer.
 */
int16_t op1_imu_read_gyro_z() {
	return softi2c_read_reg_hl(OP1_I2C2, IMU_ADDR, 0x27, 0x26);
}

/**
 * Reads the temperature from the IMU sensor connected to the I2C2 bus of OP1.
 *
 * @returns The temperature value in 16-bit signed integer format.
 */
int16_t op1_imu_read_temp() {
	return softi2c_read_reg_hl(OP1_I2C2, IMU_ADDR, 0x21, 0x20);
}
