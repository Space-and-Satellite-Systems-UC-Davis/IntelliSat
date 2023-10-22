/*
 * QMC5883L.c  (MAG interface)
 *
 *  - Apr 29, 2023 (Creation)
 *      Author       : Tim S.
 *      Contributors : nithinsenthil , Raphael
 *      Log          : Magnetometer Control functions written
 */


#include "QMC5883L.h"

void op1_mag_ctrl(int rate, int scale, int oversample) {
	if (rate) {
		softi2c_write_reg(OP1_I2C2, MAG_ADDR, 0x09, oversample << 6 | scale << 4 | (rate-1) << 2 | 1);
	} else {
		softi2c_write_reg(OP1_I2C2, MAG_ADDR, 0x09, 0);
	}
}

void op1_mag_init(int rate, int scale, int oversample) {
	softi2c_write_reg(OP1_I2C2, MAG_ADDR, 0x0A, 0x80); // soft reset
	nop(50);
	softi2c_write_reg(OP1_I2C2, MAG_ADDR, 0x0A, 0x00);
	softi2c_write_reg(OP1_I2C2, MAG_ADDR, 0x0B, 1);
	nop(50);
	op1_mag_ctrl(rate, scale, oversample);
}

int16_t op1_mag_read_x() {
	return softi2c_read_reg_hl(OP1_I2C2, MAG_ADDR, 0x01, 0x00);
}

int16_t op1_mag_read_y() {
	return softi2c_read_reg_hl(OP1_I2C2, MAG_ADDR, 0x03, 0x02);
}

int16_t op1_mag_read_z() {
	return softi2c_read_reg_hl(OP1_I2C2, MAG_ADDR, 0x05, 0x04);
}

int16_t op1_mag_read_temp() {
	return softi2c_read_reg_hl(OP1_I2C2, MAG_ADDR, 0x08, 0x07);
}
