/*
 * MB85RS256B.h
 *
 *  Created on: Sep 3, 2024
 *      Author: green
 */

#ifndef PERIPHERALS_FRAM_MB85RS256B_H_
#define PERIPHERALS_FRAM_MB85RS256B_H_

#include "../../system_config/SPI/spi.h"
#include "print_scan.h"

//Registers
#define FRAM_SPI            SPI1
#define FRAM_SPI_CS         SPI1_CS

//FRAM Op-codes. SR = Status Register
#define FRAM_READ_ID        0x9F
#define FRAM_READ_SR        0x05
#define FRAM_WRITE_SR       0x01
#define FRAM_WRITE_ENABLE   0x06
#define FRAM_READ           0x03
#define FRAM_WRITE          0x02

//Insert explanation here
void FRAM_read_deviceID(uint8_t* MISO);

/**
 * Reads one sector.
 *
 * @param sector   # of sector (0 ~ 4095) to read from. Do not use address.
 * @param buffer   Buffer to put data into. Ensure appropriate size (4096 bytes).
 *
 * @returns Whether read was successfully completed.
 */


bool FRAM_readData(uint32_t address, uint8_t* buffer);

#endif /* PERIPHERALS_FRAM_MB85RS256B_H_ */
