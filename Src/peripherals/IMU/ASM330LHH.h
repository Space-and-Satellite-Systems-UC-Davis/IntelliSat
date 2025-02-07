/*
 * ASM330LHH.h  (IMU interface)
 *
 *  - Apr 29, 2023 (Creation)
 *      Author       : Tim S.
 *      Contributors : nithinsenthil , Raphael
 *      Log          : IMU Control functions written
 */

#ifndef REALOP1_PERIPHERALS_IMU_H_
#define REALOP1_PERIPHERALS_IMU_H_

#include <I2C/i2c.h>
#include <SPI/spi.h>
#include <print_scan.h>
//Macros
#if   OP_REV == 1
#define IMU_I2C GPIOF, 1, GPIOF, 0
#elif OP_REV == 2 //TODO: Make actual change for Rev3

#define IMU_SPI    SPI3
#define IMU_SPI_CS SPI3_CS

#elif OP_REV == 3

#define IMU1_SPI    SPI2
#define IMU1_SPI_CS SPI2_CS
#define IMU0_SPI    SPI3
#define IMU0_SPI_CS SPI3_CS

enum IMU_SELECT { IMU0, IMU1 };

extern enum IMU_SELECT IMU_global;

void set_IMU(enum IMU_SELECT select);

#endif

//Macros
#define IMU_ADDR 0x6A
#define IMU_ODR_OFF 		0
#define IMU_ODR_12_5_Hz 	1
#define IMU_ODR_26_Hz 		2
#define IMU_ODR_52_Hz 		3
#define IMU_ODR_104_Hz 		4
#define IMU_ODR_208_Hz 		5
#define IMU_ODR_417_Hz 		6
#define IMU_ODR_833_Hz 		7
#define IMU_ODR_1667_Hz 	8
#define IMU_ODR_3333_Hz 	9
#define IMU_ODR_6667_Hz 	10
#define IMU_FS_2_g 			0
#define IMU_FS_4_g 			2
#define IMU_FS_8_g 			3
#define IMU_FS_16_g 		1
#define IMU_FS_125_dps 		2
#define IMU_FS_250_dps 		0
#define IMU_FS_500_dps 		4
#define IMU_FS_1000_dps 	8
#define IMU_FS_2000_dps 	12
#define IMU_FS_4000_dps 	1

#define IMU_SPI_RW 0x80 // 1 is read and 0 is write. By default spi uses MSB first, meaning bit 7 of the address is the first bit sent out, in contrast to 5.1.2 figure 4
#define IMU_SPI_DATA_DI_Msk 0x7F // data DI(7:0) (write mode). This is the data that is written into the device (MSb first)

#define IMU_CTRL1_XL_ODR_Msk 0xFF // Accelerometer ODR selection
#define IMU_CTRL1_XL_FS_Msk 0xF // Accelerometer full-scale selection
#define IMU_CTRL1_XL_LPF2_XL_EN 0x1 // Accelerometer high-resolution selection

#define IMU_CTRL1_XL_ODR_SPAN 4U
#define IMU_CTRL1_XL_FS_SPAN 2U
#define IMU_CTRL1_XL_LPF2_XL_SPAN 1U

#define IMU_CTRL2_G_Msk 0xFF // Gyroscope control register 2 (r/w)

#define OUT_TEMP_L_Pos 0x20
#define OUT_TEMP_H_Pos 0x21

#define OUTX_L_G_Pos 0x22
#define OUTX_H_G_Pos 0x23

#define OUTY_L_G_Pos 0x24
#define OUTY_H_G_Pos 0x25

#define OUTZ_L_G_Pos 0x26
#define OUTZ_H_G_Pos 0x27

#define OUTX_L_A_Pos 0x28
#define OUTX_H_A_Pos 0x29

#define OUTY_L_A_Pos 0x2A
#define OUTY_H_A_Pos 0x2B

#define OUTZ_L_A_Pos 0x2C
#define OUTZ_H_A_Pos 0x2D

/**
 * Configures the accelerometer control register of an IMU device.
 *
 * @param acel_rate The rate of the accelerometer.
 * @param acel_scale The scale of the accelerometer.
 * @param digital_filter_on Whether the digital filter is enabled or not.
 *
 * @returns None
 */
void imu_acelCtrl(int acel_rate, int acel_scale, int digital_filter_on);

/**
 * Configures the gyroscope rate and scale for the IMU sensor.
 *
 * @param gyro_rate The rate of the gyroscope.
 * @param gyro_scale The scale of the gyroscope.
 *
 * @returns None
 */
void imu_gyroCtrl(int gyro_rate, int gyro_scale);

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
void imu_init(int acel_rate, int acel_scale, int gyro_rate, int gyro_scale);

/**
 * Reads the X-axis acceleration value from the IMU sensor connected to the I2C2 bus of OP1.
 *
 * @returns The X-axis acceleration value as a 16-bit signed integer.
 */
float imu_readAcel_X();

/**
 * Reads the y-axis acceleration value from the IMU sensor connected to the I2C2 bus of OP1.
 *
 * @returns The y-axis acceleration value as a 16-bit signed integer.
 */
float imu_readAcel_Y();

/**
 * Reads the acceleration value of the z-axis from the IMU sensor connected to the I2C2 bus of OP1.
 *
 * @returns The acceleration value of the z-axis.
 */
float imu_readAcel_Z();

/**
 * Reads the X-axis gyroscope data from the IMU sensor connected to the I2C2 bus of OP1.
 *
 * @returns The X-axis gyroscope data as a 16-bit signed integer.
 */
float imu_readGyro_X();

/**
 * Reads the y-axis gyroscope value from the IMU sensor connected to the I2C2 bus of OP1.
 *
 * @returns The y-axis gyroscope value as a 16-bit signed integer.
 */
float imu_readGyro_Y();

/**
 * Reads the Z-axis gyroscope data from the IMU sensor connected to the I2C2 bus of OP1.
 *
 * @returns The Z-axis gyroscope data as a 16-bit signed integer.
 */
float imu_readGyro_Z();

/**
 * Reads the temperature from the IMU sensor connected to the I2C2 bus of OP1.
 *
 * @returns The temperature value in 16-bit signed integer format.
 */
float imu_readTemp();

/**
 * Tests SPI communication using WHO_AM_I register
 * 
 * @returns if communication between PFC and IMU are working
 */
bool imu_isCommunicationWorking();

/**
 * Prints all values in gyro, accceleromter, and temperarture
 */
void imu_printAllValues(); 

#endif /* REALOP1_PERIPHERALS_IMU_H_ */
