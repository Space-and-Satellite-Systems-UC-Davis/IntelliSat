/*
 * ASM330LHH.c  (IMU interface)
 *
 *  - Apr 29, 2023 (Creation)
 *      Author       : Tim S.
 *      Contributors : nithinsenthil , Raphael
 *      Log          : IMU Control functions written
 */

#include "ASM330LHH.h"

void op1_imu_acel_ctrl(int acel_rate, int acel_scale, int digital_filter_on) {
	acel_rate &= 0xFF;
	acel_scale &= 0xF;
	digital_filter_on &= 1;
	int data = acel_rate << 4 | acel_scale << 2 | digital_filter_on << 1;
	softi2c_write_reg(OP1_I2C2, IMU_ADDR, 0x10, data);
}

void op1_imu_gyro_ctrl(int gyro_rate, int gyro_scale) {
	gyro_rate &= 0xFF;
	gyro_scale &= 0xFF;
	int data = gyro_rate << 4 | gyro_scale;
	softi2c_write_reg(OP1_I2C2, IMU_ADDR, 0x11, data);
}

void op1_imu_init(int acel_rate, int acel_scale, int gyro_rate, int gyro_scale) {
	softi2c_write_reg(OP1_I2C2, IMU_ADDR, 0x12, 0x01); // soft reset imu
	nop(100);
	op1_imu_acel_ctrl(acel_rate, acel_scale, 0);
	op1_imu_gyro_ctrl(gyro_rate, gyro_scale);
}

int16_t op1_imu_read_acel_x() {
	return softi2c_read_reg_hl(OP1_I2C2, IMU_ADDR, 0x29, 0x28);
}

int16_t op1_imu_read_acel_y() {
	return softi2c_read_reg_hl(OP1_I2C2, IMU_ADDR, 0x2B, 0x2A);
}

int16_t op1_imu_read_acel_z() {
	return softi2c_read_reg_hl(OP1_I2C2, IMU_ADDR, 0x2D, 0x2C);
}

int16_t op1_imu_read_gyro_x() {
	return softi2c_read_reg_hl(OP1_I2C2, IMU_ADDR, 0x23, 0x22);
}

int16_t op1_imu_read_gyro_y() {
	return softi2c_read_reg_hl(OP1_I2C2, IMU_ADDR, 0x25, 0x24);
}

int16_t op1_imu_read_gyro_z() {
	return softi2c_read_reg_hl(OP1_I2C2, IMU_ADDR, 0x27, 0x26);
}

int16_t op1_imu_read_temp() {
	return softi2c_read_reg_hl(OP1_I2C2, IMU_ADDR, 0x21, 0x20);
}
