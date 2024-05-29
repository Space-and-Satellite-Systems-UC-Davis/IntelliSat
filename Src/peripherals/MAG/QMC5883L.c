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

/*************************** MAG Parameters *************************/

uint16_t mag_magFullScale = 0;

/*************************** MAG Helper Functions *************************/

#define ScaledData(data, scale) ((float)(data) * (scale) / (uint16_t)(-1))

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

    switch (scale) {
        case MAG_FS_2_G:
            mag_magFullScale = 2;
            break;
        case MAG_FS_8_G:
            mag_magFullScale = 8;
            break;
        default:
            mag_magFullScale = 0;
            break;
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
    int16_t x = softi2c_readRegHighLow(MAG_I2C, MAG_ADDR, 0x01, 0x00);
	return ScaledData(x, mag_magFullScale);
}

int16_t mag_read_Y() {
    int16_t y = softi2c_readRegHighLow(MAG_I2C, MAG_ADDR, 0x03, 0x02);
    return ScaledData(y, mag_magFullScale);
}

int16_t mag_read_Z() {
    int16_t z = softi2c_readRegHighLow(MAG_I2C, MAG_ADDR, 0x05, 0x04);
    return ScaledData(z, mag_magFullScale);
}

int16_t mag_readTemp() {
    int16_t temp = softi2c_readRegHighLow(MAG_I2C, MAG_ADDR, 0x07, 0x06);
	return temp;
}
