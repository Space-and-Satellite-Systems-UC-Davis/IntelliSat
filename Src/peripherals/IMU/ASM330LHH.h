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

//Macros
#if   OP_REV == 1
#define IMU_I2C GPIOF, 1, GPIOF, 0
#elif OP_REV == 2
#define IMU_SPI    SPI3
#define IMU_SPI_CS SPI3_CS
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
int16_t imu_readAcel_X();

/**
 * Reads the y-axis acceleration value from the IMU sensor connected to the I2C2 bus of OP1.
 *
 * @returns The y-axis acceleration value as a 16-bit signed integer.
 */
int16_t imu_readAcel_Y();

/**
 * Reads the acceleration value of the z-axis from the IMU sensor connected to the I2C2 bus of OP1.
 *
 * @returns The acceleration value of the z-axis.
 */
int16_t imu_readAcel_Z();

/**
 * Reads the X-axis gyroscope data from the IMU sensor connected to the I2C2 bus of OP1.
 *
 * @returns The X-axis gyroscope data as a 16-bit signed integer.
 */
int16_t imu_readGyro_X();

/**
 * Reads the y-axis gyroscope value from the IMU sensor connected to the I2C2 bus of OP1.
 *
 * @returns The y-axis gyroscope value as a 16-bit signed integer.
 */
int16_t imu_readGyro_Y();

/**
 * Reads the Z-axis gyroscope data from the IMU sensor connected to the I2C2 bus of OP1.
 *
 * @returns The Z-axis gyroscope data as a 16-bit signed integer.
 */
int16_t imu_readGyro_Z();

/**
 * Reads the temperature from the IMU sensor connected to the I2C2 bus of OP1.
 *
 * @returns The temperature value in 16-bit signed integer format.
 */
int16_t imu_readTemp();

#endif /* REALOP1_PERIPHERALS_IMU_H_ */
