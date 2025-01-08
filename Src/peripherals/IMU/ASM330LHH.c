/*
 * ASM330LHH.c  (IMU interface)
 *
 *	- Jan 30, 2024
 *		Author 		 : Basti
 *		Log			 : Utilized the SPI function
 *
 *	- Jan  6, 2024
 *		Author		 : Darsh
 *		Log			 : Edited function names to follow consistent naming
 *
 *  - Apr 29, 2023 (Creation)
 *      Author       : Tim S.
 *      Contributors : nithinsenthil , Raphael
 *      Log          : IMU Control functions written
 */

#include "ASM330LHH.h"

//GLOBAL VARIABLES
#define ACCEL_RATE_REG 0x10
#define GYRO_CTRL_REG  0x11
#define IMU_RESET_REG  0x12

// Sensitivity Values
#define ASM330LHH_ACC_SENSITIVITY_FS_2G   		0.061f
#define ASM330LHH_ACC_SENSITIVITY_FS_4G   		0.122f
#define ASM330LHH_ACC_SENSITIVITY_FS_8G   		0.244f
#define ASM330LHH_ACC_SENSITIVITY_FS_16G  		0.488f
#define ASM330LHH_GYRO_SENSITIVITY_FS_125DPS    4.370f
#define ASM330LHH_GYRO_SENSITIVITY_FS_250DPS    8.750f
#define ASM330LHH_GYRO_SENSITIVITY_FS_500DPS   17.500f
#define ASM330LHH_GYRO_SENSITIVITY_FS_1000DPS  35.000f
#define ASM330LHH_GYRO_SENSITIVITY_FS_2000DPS  70.000f
#define ASM330LHH_GYRO_SENSITIVITY_FS_4000DPS 140.000f

#define IMU_RESET_CMD  0x01

/****************************** IMU Properties ****************************/

int imu_acelFullScale = 0;
int imu_gyroFullScale = 0;

enum IMU_SELECT IMU_global = IMU0;

/*************************** IMU Helper Functions *************************/

// #define ScaledData(data, scale) ((data * scale) * (uint16_t)(-1))
#define ScaledData(data, scale) ((data * 1))

#if OP_REV == 3

void set_IMU(enum IMU_SELECT select){
	IMU_global = select;
}

#endif

void imu_spiWriteReg(void *address, uint8_t data) {
	uint8_t spiDATA[2];
	spiDATA[0] = (uint8_t)address & IMU_SPI_DATA_DI_Msk;
	spiDATA[1] = data;

#if OP_REV == 2
	
	spi_startCommunication(IMU_SPI_CS);
	spi_transmitReceive(IMU_SPI, spiDATA, NULL, 2, false);
	spi_stopCommunication(IMU_SPI_CS);

#elif OP_REV == 3

	if (IMU_global == IMU0) {
		spi_startCommunication(IMU0_SPI_CS);
		spi_transmitReceive(IMU0_SPI, spiDATA, NULL, 2, false);
		spi_stopCommunication(IMU0_SPI_CS);
	} else {
		spi_startCommunication(IMU1_SPI_CS);
		spi_transmitReceive(IMU1_SPI, spiDATA, NULL, 2, false);
		spi_stopCommunication(IMU1_SPI_CS);
	}
#endif

}

int16_t imu_spiReadHighLow(void *low_address) {
	uint8_t instruction = (uint8_t)low_address | IMU_SPI_RW;
	uint8_t datah, datal;

#if OP_REV == 2

	spi_startCommunication(IMU_SPI_CS);
	spi_transmitReceive(IMU_SPI, &instruction, NULL, 1, false);
	spi_transmitReceive(IMU_SPI, NULL, &datal, 1, false);
	spi_transmitReceive(IMU_SPI, NULL, &datah, 1, false);
	spi_stopCommunication(IMU_SPI_CS);

#elif OP_REV == 3

	if (IMU_global == IMU0) {
		spi_startCommunication(IMU0_SPI_CS);
		spi_transmitReceive(IMU0_SPI, &instruction, NULL, 1, false);
		spi_transmitReceive(IMU0_SPI, NULL, &datal, 1, false);
		spi_transmitReceive(IMU0_SPI, NULL, &datah, 1, false);
		spi_stopCommunication(IMU0_SPI_CS);
	} else {
		spi_startCommunication(IMU1_SPI_CS);
		spi_transmitReceive(IMU1_SPI, &instruction, NULL, 1, false);
		spi_transmitReceive(IMU1_SPI, NULL, &datal, 1, false);
		spi_transmitReceive(IMU1_SPI, NULL, &datah, 1, false);
		spi_stopCommunication(IMU1_SPI_CS);
	}

#endif

	nop(10);
	printMsg("High: %d, Low: %d\r\n",datah, datal);
	return (datah << 8) | datal;
}

/**
 * Configures the accelerometer control register of an IMU device.
 *
 * @param acel_rate The rate of the accelerometer.
 * @param acel_scale The scale of the accelerometer.
 * @param digital_filter_on Whether the digital filter is enabled or not.
 *
 * @returns None
 */
void imu_acelCtrl(int acel_rate, int acel_scale, int digital_filter_on) {

	// local variables
	acel_rate &= IMU_CTRL1_XL_ODR_Msk;
	acel_scale &= IMU_CTRL1_XL_FS_Msk;
	digital_filter_on &= IMU_CTRL1_XL_LPF2_XL_EN;
	int data = acel_rate << IMU_CTRL1_XL_ODR_SPAN | acel_scale << IMU_CTRL1_XL_FS_SPAN | digital_filter_on << IMU_CTRL1_XL_LPF2_XL_SPAN;

#if OP_REV == 1

	softi2c_writeReg(IMU_I2C, IMU_ADDR, ACCEL_RATE_REG, data);

#elif OP_REV == 2 || OP_REV == 3
	set_IMU(IMU0);
	imu_spiWriteReg(ACCEL_RATE_REG, data);
	set_IMU(IMU1);
	imu_spiWriteReg(ACCEL_RATE_REG, data);

#endif

    switch (acel_scale) {
        case IMU_FS_2_g:
            imu_acelFullScale = 2;
            break;
        case IMU_FS_4_g:
            imu_acelFullScale = 4;
            break;
        case IMU_FS_8_g:
            imu_acelFullScale = 8;
            break;
        case IMU_FS_16_g:
            imu_acelFullScale = 16;
            break;
    }

}

/**
 * Configures the gyroscope rate and scale for the IMU sensor.
 *
 * @param gyro_rate The rate of the gyroscope.
 * @param gyro_scale The scale of the gyroscope.
 *
 * @returns None
 */
void imu_gyroCtrl(int gyro_rate, int gyro_scale) {

	// local variables
	gyro_rate &= IMU_CTRL2_G_Msk;
	gyro_scale &= IMU_CTRL2_G_Msk;
	int data = gyro_rate << IMU_CTRL1_XL_ODR_SPAN | gyro_scale;

#if OP_REV == 1

	softi2c_writeReg(IMU_I2C, IMU_ADDR, GYRO_CTRL_REG, data);

#elif OP_REV == 2 || OP_REV == 3


	set_IMU(IMU0);
	imu_spiWriteReg(GYRO_CTRL_REG, data);
	set_IMU(IMU1);
	imu_spiWriteReg(GYRO_CTRL_REG, data);

#endif

    switch (gyro_scale) {
        case IMU_FS_125_dps:
            imu_gyroFullScale = 125;
            break;
        case IMU_FS_250_dps:
            imu_gyroFullScale = 250;
            break;
        case IMU_FS_500_dps:
            imu_gyroFullScale = 500;
            break;
        case IMU_FS_1000_dps:
            imu_gyroFullScale = 1000;
            break;
        case IMU_FS_2000_dps:
            imu_gyroFullScale = 2000;
            break;
        case IMU_FS_4000_dps:
            imu_gyroFullScale = 4000;
            break;
    }
}

/*************************** IMU Interface Functions *************************/

void imu_init(int acel_rate, int acel_scale, int gyro_rate, int gyro_scale) {

#if OP_REV == 1

	softi2c_init(IMU_I2C);
	softi2c_writeReg(IMU_I2C, IMU_ADDR, IMU_RESET_REG, IMU_RESET_CMD); // soft reset imu

#elif OP_REV == 2

	spi_config(IMU_SPI);
	imu_spiWriteReg(IMU_RESET_REG, IMU_RESET_CMD);

#elif OP_REV == 3

	set_IMU(IMU1);

	spi_config(IMU1_SPI);
	imu_spiWriteReg(IMU_RESET_REG, IMU_RESET_CMD);

	set_IMU(IMU0);

	spi_config(IMU0_SPI);
	imu_spiWriteReg(IMU_RESET_REG, IMU_RESET_CMD);

#endif

	//initialize accelerometer and gyroscope
	imu_acelCtrl(acel_rate, acel_scale, 0);
	imu_gyroCtrl(gyro_rate, gyro_scale);


}

int16_t imu_readAcel_X() {

	uint8_t instructionHi = OUTX_H_A_Pos | IMU_SPI_RW ;	//Where we send Hi instruction
	uint8_t instructionLow = OUTX_L_A_Pos | IMU_SPI_RW;	//Where we send Low instruction

    int16_t data = 0;

#if OP_REV == 1

	data = softi2c_readRegHighLow(IMU_I2C, IMU_ADDR, instructionHi, instructionLow);

#elif OP_REV == 2 || OP_REV == 3

	data =  imu_spiReadHighLow((uint8_t*)0x28);

#endif

    return ScaledData(data, imu_acelFullScale);

}

int16_t imu_readAcel_Y() {

	uint8_t instructionHi = OUTY_H_A_Pos | IMU_SPI_RW;	//Where we send Hi instruction
	uint8_t instructionLow = OUTY_L_A_Pos | IMU_SPI_RW;	//Where we send Low instruction

    int16_t data = 0;

#if OP_REV == 1

	data = softi2c_readRegHighLow(IMU_I2C, IMU_ADDR, instructionHi, instructionLow);

#elif OP_REV == 2 || OP_REV == 3

	data = imu_spiReadHighLow((uint8_t*)0x2A);

#endif

    return ScaledData(data, imu_acelFullScale);

}

int16_t imu_readAcel_Z() {

	uint8_t instructionHi = OUTZ_H_A_Pos | IMU_SPI_RW;	//Where we send Hi instruction
	uint8_t instructionLow = OUTZ_L_A_Pos | IMU_SPI_RW;	//Where we send Low instruction

    int16_t data = 0;

#if OP_REV == 1

	data = softi2c_readRegHighLow(IMU_I2C, IMU_ADDR, instructionHi, instructionLow);

#elif OP_REV == 2 || OP_REV == 3

	data = imu_spiReadHighLow((uint8_t*)0x2C);

#endif

    return ScaledData(data, imu_acelFullScale);
}

int16_t imu_readGyro_X() {

	uint8_t instructionHi = OUTX_H_G_Pos | IMU_SPI_RW;	//Where we send Hi instruction
	uint8_t instructionLow = OUTX_L_G_Pos | IMU_SPI_RW;	//Where we send Low instruction

    int16_t data = 0;

	#if OP_REV == 1

	data = softi2c_readRegHighLow(IMU_I2C, IMU_ADDR, instructionHi, instructionLow);

#elif OP_REV == 2 || OP_REV == 3

	data = imu_spiReadHighLow((uint8_t*)0x22);

#endif

    return ScaledData(data, imu_gyroFullScale);
}

int16_t imu_readGyro_Y() {

	uint8_t instructionHi = OUTY_H_G_Pos | IMU_SPI_RW;	//Where we send Hi instruction
	uint8_t instructionLow = OUTY_L_G_Pos | IMU_SPI_RW;	//Where we send Low instruction

    int16_t data = 0;

#if OP_REV == 1

	data = softi2c_readRegHighLow(IMU_I2C, IMU_ADDR, instructionHi, instructionLow);

#elif OP_REV == 2 || OP_REV == 3

	data = imu_spiReadHighLow((uint8_t*)0x24);

#endif

    return ScaledData(data, imu_gyroFullScale);
}

int16_t imu_readGyro_Z() {
	uint8_t instructionHi = OUTZ_H_G_Pos | IMU_SPI_RW;	//Where we send Hi instruction
	uint8_t instructionLow = OUTZ_L_G_Pos | IMU_SPI_RW;	//Where we send Low instruction

    int16_t data = 0;

#if OP_REV == 1

	data = softi2c_readRegHighLow(IMU_I2C, IMU_ADDR, instructionHi, instructionLow);

#elif OP_REV == 2 || OP_REV == 3

	data = imu_spiReadHighLow((uint8_t*)0x26);

#endif

    return ScaledData(data, imu_gyroFullScale);
}

int16_t imu_readTemp() {

	uint8_t instructionHi = OUT_TEMP_H_Pos | IMU_SPI_RW;	//Where we send Hi instruction
	uint8_t instructionLow = OUT_TEMP_L_Pos | IMU_SPI_RW;	//Where we send Low instruction

    int16_t data = 0;

#if OP_REV == 1

	data = softi2c_readRegHighLow(IMU_I2C, IMU_ADDR, instructionHi, instructionLow);

#elif OP_REV == 2 || OP_REV == 3

	data = imu_spiReadHighLow((uint8_t*)0x20);

#endif

    return data;
}
