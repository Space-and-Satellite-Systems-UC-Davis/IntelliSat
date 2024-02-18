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
#define ASM330LHH_ACC_SENSITIVITY_FS_2G   0.061f
#define ASM330LHH_ACC_SENSITIVITY_FS_4G   0.122f
#define ASM330LHH_ACC_SENSITIVITY_FS_8G   0.244f
#define ASM330LHH_ACC_SENSITIVITY_FS_16G  0.488f

#define ASM330LHH_GYRO_SENSITIVITY_FS_125DPS    4.370f
#define ASM330LHH_GYRO_SENSITIVITY_FS_250DPS    8.750f
#define ASM330LHH_GYRO_SENSITIVITY_FS_500DPS   17.500f
#define ASM330LHH_GYRO_SENSITIVITY_FS_1000DPS  35.000f
#define ASM330LHH_GYRO_SENSITIVITY_FS_2000DPS  70.000f
#define ASM330LHH_GYRO_SENSITIVITY_FS_4000DPS 140.000f



#define IMU_RESET_CMD  0x01

/*************************** IMU Helper Functions *************************/

/**
 * Configures the accelerometer control register of an IMU device.
 *
 * @param acel_rate The rate of the accelerometer.
 * @param acel_scale The scale of the accelerometer.
 * @param digital_filter_on Whether the digital filter is enabled or not.
 *
 * @returns None
 */
//void imu_acelCtrl(int acel_rate, int acel_scale, int digital_filter_on) {
//
//	// local variables
//	acel_rate &= 0xFF;
//	acel_scale &= 0xF;
//	digital_filter_on &= 1;
//	int data = acel_rate << 4 | acel_scale << 2 | digital_filter_on << 1;
//
//#if OP_REV == 1
//
//	softi2c_writeReg(IMU_I2C, IMU_ADDR, ACCEL_RATE_REG, data);
//
//#elif OP_REV == 2
//
//	spi_startCommunication(IMU_SPI_CS);
//
//	uint8_t spiDATA[2];
//	spiDATA[0] = ACCEL_RATE_REG & 0x7F;		//address of register
//	spiDATA[1] = data;						//data to be written in register
//
//	//transmit
//	spi_transmitReceive(IMU_SPI, spiDATA, NULL, 2, false);
//	spi_stopCommunication(IMU_SPI_CS);
//
//#endif
//}

/**
 * Configures the gyroscope rate and scale for the IMU sensor.
 *
 * @param gyro_rate The rate of the gyroscope.
 * @param gyro_scale The scale of the gyroscope.
 *
 * @returns None
 */
//void imu_gyroCtrl(int gyro_rate, int gyro_scale) {
//
//	// local variables
//	gyro_rate &= 0xFF;
//	gyro_scale &= 0xFF;
//	int data = gyro_rate << 4 | gyro_scale;
//
//#if OP_REV == 1
//
//	softi2c_writeReg(IMU_I2C, IMU_ADDR, GYRO_CTRL_REG, data);
//
//#elif OP_REV == 2
//
//	spi_startCommunication(IMU_SPI_CS);
//
//	uint8_t spiDATA[2];
//	spiDATA[0] = GYRO_CTRL_REG & 0x7F;		//address of register
//	spiDATA[1] = data;						//data to be written in register
//
//	//transmit
//	spi_transmitReceive(IMU_SPI, spiDATA, NULL, 2, false);
//	spi_stopCommunication(IMU_SPI_CS);
//
//#endif
//}

/*************************** IMU Interface Functions *************************/

//void imu_init(int acel_rate, int acel_scale, int gyro_rate, int gyro_scale) {
//
//#if OP_REV == 1
//
//	softi2c_init(IMU_I2C);
//
//	softi2c_writeReg(IMU_I2C, IMU_ADDR, IMU_RESET_REG, IMU_RESET_CMD); // soft reset imu
//	nop(100);
//	imu_acelCtrl(acel_rate, acel_scale, 0);
//	imu_gyroCtrl(gyro_rate, gyro_scale);
//
//#elif OP_REV == 2
//
//	spi_config(IMU_SPI);
//
//	spi_startCommunication(IMU_SPI_CS);
//
//	uint8_t resetDATA[2];
//	resetDATA[0] = IMU_RESET_REG & 0x7F;	//address of register to soft reset
//	resetDATA[1] = IMU_RESET_CMD;			//to reset
//
//	//soft reset imu
//	//transmit
//	spi_transmitReceive(IMU_SPI, resetDATA, NULL, 2, false);
//
//
//
//	spi_stopCommunication(IMU_SPI_CS);
//	nop(100);	//delay
//
//	//initialize
//	imu_acelCtrl(acel_rate, acel_scale, 0);
//	imu_gyroCtrl(gyro_rate, gyro_scale);
//
//#endif
//}

int16_t imu_readAcel_X() {

	uint8_t instructionHi = 0x29 | 0x80 ;	//Where we send Hi instruction
	uint8_t storeInstructionHi;		//Where we receive Hi instruction

	uint8_t instructionLow = 0x28 | 0x80;	//Where we send Low instruction
	uint8_t storeInstructionLow;	//Where we receive Low instruction

#if OP_REV == 1

	return softi2c_readRegHighLow(IMU_I2C, IMU_ADDR, instructionHi, instructionLow);

#elif OP_REV == 2

	spi_startCommunication(IMU_SPI_CS);

	//transmit
	spi_transmitReceive(IMU_SPI, &instructionHi, NULL, 1, false);

	//receive
	spi_transmitReceive(IMU_SPI, NULL, &storeInstructionHi, 1, false);

	//transmit
	spi_transmitReceive(IMU_SPI, &instructionLow, NULL, 1, false);

	//receive
	spi_transmitReceive(IMU_SPI, NULL, &storeInstructionLow, 1, false);

	//Or the Hi and Low to get 16 bits
	int16_t Result = (storeInstructionHi << 8 | storeInstructionLow);

	spi_stopCommunication(IMU_SPI_CS);

	return Result;

#endif
}

//int16_t imu_readAcel_Y() {
//
//	uint8_t instructionHi = 0x2B;	//Where we send Hi instruction
//	uint8_t storeInstructionHi;		//Where we receive Hi instruction
//
//	uint8_t instructionLow = 0x2A;	//Where we send Low instruction
//	uint8_t storeInstructionLow;	//Where we receive Low instruction
//
//#if OP_REV == 1
//
//	return softi2c_readRegHighLow(IMU_I2C, IMU_ADDR, instructionHi, instructionLow);
//
//#elif OP_REV == 2
//
//	spi_startCommunication(IMU_SPI_CS);
//
//	//transmit
//	spi_transmitReceive(IMU_SPI, &instructionHi, NULL, 1, false);
//
//	//receive
//	spi_transmitReceive(IMU_SPI, NULL, &storeInstructionHi, 1, false);
//
//	//transmit
//	spi_transmitReceive(IMU_SPI, &instructionLow, NULL, 1, false);
//
//	//receive
//	spi_transmitReceive(IMU_SPI, NULL, &storeInstructionLow, 1, false);
//
//	//Or the Hi and Low to get 16 bits
//	int16_t Result = (instructionHi << 8 | storeInstructionLow);
//
//	spi_stopCommunication(IMU_SPI_CS);
//
//	return Result;
//
//#endif
//}

//int16_t imu_readAcel_Z() {
//
//	uint8_t instructionHi = 0x2D;	//Where we send Hi instruction
//	uint8_t storeInstructionHi;		//Where we receive Hi instruction
//
//	uint8_t instructionLow = 0x2C;	//Where we send Low instruction
//	uint8_t storeInstructionLow;	//Where we receive Low instruction
//
//#if OP_REV == 1
//
//	return softi2c_readRegHighLow(IMU_I2C, IMU_ADDR, instructionHi, instructionLow);
//
//#elif OP_REV == 2
//
//	spi_startCommunication(IMU_SPI_CS);
//
//	//transmit
//	spi_transmitReceive(IMU_SPI, &instructionHi, NULL, 1, false);
//
//	//receive
//	spi_transmitReceive(IMU_SPI, NULL, &storeInstructionHi, 1, false);
//
//	//transmit
//	spi_transmitReceive(IMU_SPI, &instructionLow, NULL, 1, false);
//
//	//receive
//	spi_transmitReceive(IMU_SPI, NULL, &storeInstructionLow, 1, false);
//
//	//Or the Hi and Low to get 16 bits
//	int16_t Result = (instructionHi << 8 | storeInstructionLow);
//
//	spi_stopCommunication(IMU_SPI_CS);
//
//	return Result;
//
//#endif
//}

//int16_t imu_readGyro_X() {
//
//	uint8_t instructionHi = 0x23;	//Where we send Hi instruction
//	uint8_t storeInstructionHi;		//Where we receive Hi instruction
//
//	uint8_t instructionLow = 0x22;	//Where we send Low instruction
//	uint8_t storeInstructionLow;	//Where we receive Low instruction
//
//#if OP_REV == 1
//
//	return softi2c_readRegHighLow(IMU_I2C, IMU_ADDR, instructionHi, instructionLow);
//
//#elif OP_REV == 2
//
//	spi_startCommunication(IMU_SPI_CS);
//
//	//transmit
//	spi_transmitReceive(IMU_SPI, &instructionHi, NULL, 1, false);
//
//	//receive
//	spi_transmitReceive(IMU_SPI, NULL, &storeInstructionHi, 1, false);
//
//	//transmit
//	spi_transmitReceive(IMU_SPI, &instructionLow, NULL, 1, false);
//
//	//receive
//	spi_transmitReceive(IMU_SPI, NULL, &storeInstructionLow, 1, false);
//
//	int16_t Result = (instructionHi << 8 | storeInstructionLow);
//
//	spi_stopCommunication(IMU_SPI_CS);
//
//	return Result;
//
//#endif
//}

//int16_t imu_readGyro_Y() {
//
//	uint8_t instructionHi = 0x25;	//Where we send Hi instruction
//	uint8_t storeInstructionHi;		//Where we receive Hi instruction
//
//	uint8_t instructionLow = 0x24;	//Where we send Low instruction
//	uint8_t storeInstructionLow;	//Where we receive Low instruction
//
//#if OP_REV == 1
//
//	return softi2c_readRegHighLow(IMU_I2C, IMU_ADDR, instructionHi, instructionLow);
//
//#elif OP_REV == 2
//
//	spi_startCommunication(IMU_SPI_CS);
//
//	//transmit
//	spi_transmitReceive(IMU_SPI, &instructionHi, NULL, 1, false);
//
//	//receive
//	spi_transmitReceive(IMU_SPI, NULL, &storeInstructionHi, 1, false);
//
//	//transmit
//	spi_transmitReceive(IMU_SPI, &instructionLow, NULL, 1, false);
//
//	//receive
//	spi_transmitReceive(IMU_SPI, NULL, &storeInstructionLow, 1, false);
//
//	//Or the Hi and Low to get 16 bits
//	int16_t Result = (instructionHi << 8 | storeInstructionLow);
//
//	spi_stopCommunication(IMU_SPI_CS);
//
//	return Result;
//
//#endif
//}

//int16_t imu_readGyro_Z() {
//
//	uint8_t instructionHi = 0x27;	//Where we send Hi instruction
//	uint8_t storeInstructionHi;		//Where we receive Hi instruction
//
//	uint8_t instructionLow = 0x26;	//Where we send Low instruction
//	uint8_t storeInstructionLow;	//Where we receive Low instruction
//
//#if OP_REV == 1
//
//	return softi2c_readRegHighLow(IMU_I2C, IMU_ADDR, instructionHi, instructionLow);
//
//#elif OP_REV == 2
//
//	spi_startCommunication(IMU_SPI_CS);
//
//	//transmit
//	spi_transmitReceive(IMU_SPI, &instructionHi, NULL, 1, false);
//
//	//receive
//	spi_transmitReceive(IMU_SPI, NULL, &storeInstructionHi, 1, false);
//
//	//transmit
//	spi_transmitReceive(IMU_SPI, &instructionLow, NULL, 1, false);
//
//	//receive
//	spi_transmitReceive(IMU_SPI, NULL, &storeInstructionLow, 1, false);
//
//	//Or the Hi and Low to get 16 bits
//	int16_t Result = (instructionHi << 8 | storeInstructionLow);
//
//	spi_stopCommunication(IMU_SPI_CS);
//
//	return Result;
//
//#endif
//}

//int16_t imu_readTemp() {
//
//	uint8_t instructionHi = 0x21;	//Where we send Hi instruction
//	uint8_t storeInstructionHi;		//Where we receive Hi instruction
//
//	uint8_t instructionLow = 0x20;	//Where we send Low instruction
//	uint8_t storeInstructionLow;	//Where we receive Low instruction
//
//#if OP_REV == 1
//
//	return softi2c_readRegHighLow(IMU_I2C, IMU_ADDR, instructionHi, instructionLow);
//
//#elif OP_REV == 2
//
//	spi_startCommunication(IMU_SPI_CS);
//
//	//transmit
//	spi_transmitReceive(IMU_SPI, &instructionHi, NULL, 1, false);
//
//	//receive
//	spi_transmitReceive(IMU_SPI, NULL, &storeInstructionHi, 1, false);
//
//	//transmit
//	spi_transmitReceive(IMU_SPI, &instructionLow, NULL, 1, false);
//
//	//receive
//	spi_transmitReceive(IMU_SPI, NULL, &storeInstructionLow, 1, false);
//
//	//Or the Hi and Low to get 16 bits
//	int16_t Result = (instructionHi << 8 | storeInstructionLow);
//
//	spi_stopCommunication(IMU_SPI_CS);
//
//	return Result;
//
//#endif
//}
