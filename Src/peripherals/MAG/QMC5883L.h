/*
 * QMC5883L.h  (MAG interface)
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

#ifndef REALOP1_PERIPHERALS_MAG_H_
#define REALOP1_PERIPHERALS_MAG_H_


#include <I2C/i2c.h>

#if   OP_REV == 1
#define MAG_I2C GPIOF, 1, GPIOF, 0
#elif OP_REV == 2
#define MAG_I2C GPIOB, 8, GPIOB, 9
#endif

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
 * Initializes the magnetometer for operation.
 *
 * @param rate The data output rate of the magnetometer.
 * @param scale The full-scale range of the magnetometer.
 * @param oversample The oversampling rate of the magnetometer.
 *
 * @returns None
 */
void mag_init(int rate, int scale, int oversample);

/**
 * Reads the x-axis magnetometer value from the OP1 sensor.
 *
 * @returns The x-axis magnetometer value.
 */
int16_t mag_read_X();

/**
 * Reads the Y-axis magnetometer value from the OP1 sensor.
 *
 * @returns The Y-axis magnetometer value as a 16-bit integer.
 */
int16_t mag_read_Y();

/**
 * Reads the magnitude value of the z-axis from the magnetometer connected to the I2C2 bus of OP1.
 *
 * @returns The magnitude value of the z-axis.
 */
int16_t mag_read_Z();

/**
 * Reads the temperature from the magnetometer connected to OP1 via I2C2.
 *
 * @returns The temperature value in 16-bit signed integer format.
 */
int16_t mag_readTemp();


#endif /* REALOP1_PERIPHERALS_MAG_H_ */
