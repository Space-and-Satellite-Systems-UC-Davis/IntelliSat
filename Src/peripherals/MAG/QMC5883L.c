/*
 * QMC5883L.c  (MAG interface)
 *
 *  - Apr 29, 2023 (Creation)
 *      Author       : Tim S.
 *      Contributors : nithinsenthil , Raphael
 *      Log          : Magnetometer Control functions written
 */


#include "QMC5883L.h"

/**
 * Configures the control registers of the OP1 magnetometer.
 *
 * @param rate The output data rate of the magnetometer.
 * @param scale The full scale range of the magnetometer.
 * @param oversample The number of samples to average for each measurement.
 *
 * @returns None
 */
void op1_mag_ctrl(int rate, int scale, int oversample) {
	if (rate) {
		softi2c_write_reg(OP1_I2C2, MAG_ADDR, 0x09, oversample << 6 | scale << 4 | (rate-1) << 2 | 1);
	} else {
		softi2c_write_reg(OP1_I2C2, MAG_ADDR, 0x09, 0);
	}
}

/**
 * Initializes the magnetometer for operation.
 *
 * @param rate The data output rate of the magnetometer.
 * @param scale The full-scale range of the magnetometer.
 * @param oversample The oversampling rate of the magnetometer.
 *
 * @returns None
 */
void op1_mag_init(int rate, int scale, int oversample) {
	softi2c_write_reg(OP1_I2C2, MAG_ADDR, 0x0A, 0x80); // soft reset
	nop(50);
	softi2c_write_reg(OP1_I2C2, MAG_ADDR, 0x0A, 0x00);
	softi2c_write_reg(OP1_I2C2, MAG_ADDR, 0x0B, 1);
	nop(50);
	op1_mag_ctrl(rate, scale, oversample);
}

/**
 * Reads the x-axis magnetometer value from the OP1 sensor.
 *
 * @returns The x-axis magnetometer value.
 */
int16_t op1_mag_read_x() {
	return softi2c_read_reg_hl(OP1_I2C2, MAG_ADDR, 0x01, 0x00);
}

/**
 * Reads the Y-axis magnetometer value from the OP1 sensor.
 *
 * @returns The Y-axis magnetometer value as a 16-bit integer.
 */
int16_t op1_mag_read_y() {
	return softi2c_read_reg_hl(OP1_I2C2, MAG_ADDR, 0x03, 0x02);
}

/**
 * Reads the magnitude value of the z-axis from the magnetometer connected to the I2C2 bus of OP1.
 *
 * @returns The magnitude value of the z-axis.
 */
int16_t op1_mag_read_z() {
	return softi2c_read_reg_hl(OP1_I2C2, MAG_ADDR, 0x05, 0x04);
}

/**
 * Reads the temperature from the magnetometer connected to OP1 via I2C2.
 *
 * @returns The temperature value in 16-bit signed integer format.
 */
int16_t op1_mag_read_temp() {
	return softi2c_read_reg_hl(OP1_I2C2, MAG_ADDR, 0x08, 0x07);
}
