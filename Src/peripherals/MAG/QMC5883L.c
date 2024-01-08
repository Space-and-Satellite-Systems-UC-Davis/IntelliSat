/*
 * QMC5883L.c  (MAG interface)
 *
 *	- Jan  6, 2024
 *		Author		 : Darsh
 *		Log			 : Edited function names to follow consistent naming
 *
 *  - Apr 29, 2023 (Creation)
 *      Author       : Tim S.
 *      Contributors : nithinsenthil , Raphael
 *      Log          : Magnetometer Control functions written
 */


#include "QMC5883L.h"

/*************************** MAG Helper Functions *************************/

/**
 * Configures the control registers of the OP1 magnetometer.
 *
 * @param rate The output data rate of the magnetometer.
 * @param scale The full scale range of the magnetometer.
 * @param oversample The number of samples to average for each measurement.
 *
 * @returns None
 */
void magCtrl(int rate, int scale, int oversample) {
	if (rate) {
		softi2c_writeReg(MAG_I2C, MAG_ADDR, 0x09, oversample << 6 | scale << 4 | (rate-1) << 2 | 1);
	} else {
		softi2c_writeReg(MAG_I2C, MAG_ADDR, 0x09, 0);
	}
}

/*************************** MAG Interface Functions *************************/

void mag_init(int rate, int scale, int oversample) {
	softi2c_init(MAG_I2C);

	softi2c_writeReg(MAG_I2C, MAG_ADDR, 0x0A, 0x80); // soft reset
	nop(50);
	softi2c_writeReg(MAG_I2C, MAG_ADDR, 0x0A, 0x00);
	softi2c_writeReg(MAG_I2C, MAG_ADDR, 0x0B, 1);
	nop(50);
	magCtrl(rate, scale, oversample);
}

int16_t mag_read_X() {
	return softi2c_readRegHighLow(MAG_I2C, MAG_ADDR, 0x01, 0x00);
}

int16_t mag_read_Y() {
	return softi2c_readRegHighLow(MAG_I2C, MAG_ADDR, 0x03, 0x02);
}

int16_t mag_read_Z() {
	return softi2c_readRegHighLow(MAG_I2C, MAG_ADDR, 0x05, 0x04);
}

int16_t mag_readTemp() {
	return softi2c_readRegHighLow(MAG_I2C, MAG_ADDR, 0x08, 0x07);
}
