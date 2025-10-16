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
#define MAG_I2C_SCL GPIOF, 1
#define MAG_I2C_SDA GPIOF, 0
#elif OP_REV == 2
#define MAG_I2C_SCL GPIOB, 8
#define MAG_I2C_SDA GPIOB, 9
#elif OP_REV == 3
#define MAG0_I2C_SCL GPIOB, 8
#define MAG0_I2C_SDA GPIOB, 9
#define MAG1_I2C_SCL GPIOF, 0
#define MAG1_I2C_SDA GPIOF, 1

#endif

//Macros
#define MAG_ADDR 			0x0D

#define XOUT_LSB_ADDR       0x00 // Data Output X LSB Register
#define XOUT_MSB_ADDR       0x01 // Data Output X MSB Register
#define YOUT_LSB_ADDR       0x02 // Data Output Y LSB Register
#define YOUT_MSB_ADDR       0x03 // Data Output Y MSB Register
#define ZOUT_LSB_ADDR       0x04 // Data Output Z LSB Register
#define ZOUT_MSB_ADDR       0x05 // Data Output Z MSB Register
#define MAG_SR1_ADDR        0x06 // Status Register 1
#define TOUT_LSB_ADDR       0x07 // LSB Temperature Data Register
#define TOUT_MSB_ADDR       0x08 // MSB Temperature Data Register

#define MAG_CR1_ADDR        0x09 // Control Register 1
#define MAG_MODE_STANDBY    0
#define MAG_MODE_CONTINUOUS 1
#define MAG_ODR_OFF 		0
#define MAG_ODR_10_Hz 		0
#define MAG_ODR_50_Hz 		1
#define MAG_ODR_100_Hz 		2
#define MAG_ODR_200_Hz		3
#define MAG_ODR_SPAN        2
#define MAG_FS_2_G 			0 // btw: "G" is not a typo, it stands for gauss
#define MAG_FS_8_G	 		1
#define MAG_FS_SPAN         4
#define MAG_OVERSAMPLE_512 	0
#define MAG_OVERSAMPLE_256 	1
#define MAG_OVERSAMPLE_128 	2
#define MAG_OVERSAMPLE_64  	3
#define MAG_OVERSAMPLE_SPAN 6

#define MAG_CR2_ADDR            0x0A       // Control Register 2
#define MAG_CR2_SOFT_RST_Msk    (0x1 << 8) // INT_ENB: “0”: enable interrupt PIN, “1”: disable interrupt PIN
#define MAG_CR2_ROL_PNT_Msk     (0x1 << 7) // ROL_PNT: “0”: Normal, “1”: Enable pointer roll-over function
#define MAG_CR2_INT_ENB_Msk     (0x1 << 0) // SOFT_RST: “0”: Normal“1”: Soft reset, restore default value of all registers.

#define MAG_BSRR_ADDR           0x0B       // SET/RESET Period Register. Controlled by FBR [7:0]. It is recommended that the register 0BH is written by 0x01.
#define MAG_BSRR_RST            0x01       // Value to Hard Reset


typedef enum  {
    MAG0,
    MAG1 
} Mag_Selector;

typedef enum  {
    MAG0,
    MAG1 
} Mag_Selector;

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

/**
 * Select between mag0 and mag1
 * 
 * @param mag_picker The magnetometer to select
 */
void mag_select(Mag_Selector mag_picker);


#endif /* REALOP1_PERIPHERALS_MAG_H_ */
