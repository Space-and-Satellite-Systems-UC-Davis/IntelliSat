/*
 * MB85RS256B.h
 *
 *  Created on: Sep 3, 2024
 *      Author: 
 */

#ifndef PERIPHERALS_FRAM_MB85RS256B_H_
#define PERIPHERALS_FRAM_MB85RS256B_H_

#include "../../system_config/SPI/spi.h"
#include "print_scan.h"

// Registers
#define FRAM_SPI SPI1
#define FRAM_SPI_CS SPI1_CS

// FRAM Op-codes. SR = Status Register
#define FRAM_READ_ID 0x9F
#define FRAM_READ_SR 0x05
#define FRAM_WRITE_SR 0x01
#define FRAM_WRITE_ENABLE 0x06
#define FRAM_WRITE_DISABLE 0x04
#define FRAM_READ 0x03
#define FRAM_WRITE 0x02
#define FRAM_MAX_BYTES 0x8000

// FRAM Device ID
void FRAM_read_deviceID(uint8_t *MISO);

// FRAM Status Register
uint8_t FRAM_readStatusRegister();

// FRAM Read Functions

bool FRAM_readPage(uint16_t page, uint8_t *buffer);
bool FRAM_readData(uint16_t address, uint8_t *buffer);
bool FRAM_readSector(uint16_t sector, uint8_t *buffer);

// FRAM Write Functions
bool FRAM_writeEnable();
bool FRAM_writeDisable();
bool FRAM_writePage(uint16_t page, const uint8_t *data);
bool FRAM_writeSector(uint16_t sector, const uint8_t *data);
bool FRAM_writeData(uint16_t address, uint8_t* data, uint16_t size);

#endif /* PERIPHERALS_FRAM_MB85RS256B_H_ */
