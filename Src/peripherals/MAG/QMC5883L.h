/*
 * QMC5883L.h  (MAG interface)
 *
 *  - Apr 29, 2023 (Creation)
 *      Author       : Tim S.
 *      Contributors : nithinsenthil , Raphael
 *      Log          : Magnetometer Control functions written
 */

#ifndef REALOP1_PERIPHERALS_MAG_H_
#define REALOP1_PERIPHERALS_MAG_H_


#include "../../system_config/I2C/i2c.h"

//Macros
#define MAG_ADDR 			0x0D
#define MAG_ODR_OFF 		0
#define MAG_ODR_10_Hz 		1
#define MAG_ODR_50_Hz 		2
#define MAG_ODR_100_Hz 		3
#define MAG_ODR_200_Hz		4
#define MAG_FS_2_G 			0 // btw: "G" is not a typo, it stands for gauss
#define MAG_FS_8_G	 		1
#define MAG_OVERSAMPLE_512 	0
#define MAG_OVERSAMPLE_256 	1
#define MAG_OVERSAMPLE_128 	2
#define MAG_OVERSAMPLE_64  	3

/**
 * Configures the control registers of the OP1 magnetometer.
 *
 * @param rate The output data rate of the magnetometer.
 * @param scale The full scale range of the magnetometer.
 * @param oversample The number of samples to average for each measurement.
 *
 * @returns None
 */
void op1_mag_ctrl(int rate, int scale, int oversample);

/**
 * Initializes the magnetometer for operation.
 *
 * @param rate The data output rate of the magnetometer.
 * @param scale The full-scale range of the magnetometer.
 * @param oversample The oversampling rate of the magnetometer.
 *
 * @returns None
 */
void op1_mag_init(int rate, int scale, int oversample);

/**
 * Reads the x-axis magnetometer value from the OP1 sensor.
 *
 * @returns The x-axis magnetometer value.
 */
int16_t op1_mag_read_x();

/**
 * Reads the Y-axis magnetometer value from the OP1 sensor.
 *
 * @returns The Y-axis magnetometer value as a 16-bit integer.
 */
int16_t op1_mag_read_y();

/**
 * Reads the magnitude value of the z-axis from the magnetometer connected to the I2C2 bus of OP1.
 *
 * @returns The magnitude value of the z-axis.
 */
int16_t op1_mag_read_z();

/**
 * Reads the temperature from the magnetometer connected to OP1 via I2C2.
 *
 * @returns The temperature value in 16-bit signed integer format.
 */
int16_t op1_mag_read_temp();


#endif /* REALOP1_PERIPHERALS_MAG_H_ */
