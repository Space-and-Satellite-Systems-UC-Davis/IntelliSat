/*
 * MB85RS256B.c
 *
 *  Created on: Sep 3, 2024
 *      Author: green
 */

#include "MB85RS256B.h"
#include <stddef.h>

/*  Helper function:
    Timing issue with raising CS at the end. It was triggering before we had a chance to read all the data.
    Ensures the SPI hardware has finished transmitting and receiving data before moving on.
*/
void FRAM_wait() {
    while (FRAM_SPI->SR & SPI_SR_BSY);
}

void FRAM_read_deviceID(uint8_t *MISO)
{
    uint8_t MOSI = FRAM_READ_ID;

    spi_startCommunication(FRAM_SPI_CS);
    spi_transmitReceive(FRAM_SPI, &MOSI, NULL, 1, false);
    spi_transmitReceive(FRAM_SPI, NULL, MISO, 5, false);
    spi_stopCommunication(FRAM_SPI_CS);
}

bool FRAM_writeEnable() {
    uint8_t MOSI = FRAM_WRITE_ENABLE;

    // Activate SPI communication
    spi_startCommunication(FRAM_SPI_CS);

    // Send WRITE_ENABLE command
    bool success = spi_transmitReceive(FRAM_SPI, &MOSI, NULL, 1, false);
    FRAM_wait();

    // End communication
    spi_stopCommunication(FRAM_SPI_CS);
    return success;
}

bool FRAM_writeDisable() {
    uint8_t MOSI = FRAM_WRITE_DISABLE;

    // Activate SPI communication
    spi_startCommunication(FRAM_SPI_CS);

    // Send WRITE_DISABLE command
    bool success = spi_transmitReceive(FRAM_SPI, &MOSI, NULL, 1, false);

    // End communication
    spi_stopCommunication(FRAM_SPI_CS);
    return success;
}

uint8_t FRAM_readStatusRegister() {
    uint8_t MOSI = FRAM_READ_SR;
    uint8_t status = 0;
    uint8_t trash;

    // Activate SPI communication
    spi_startCommunication(FRAM_SPI_CS);

    // Send READ_STATUS_REGISTER command
    if (!spi_transmitReceive(FRAM_SPI, &MOSI, NULL, 1, false)) {
        spi_stopCommunication(FRAM_SPI_CS);
        return 0;  // Return 0 if command fails
    }
    // Trash the dummy byte
    spi_transmitReceive(FRAM_SPI, NULL, &trash, 1, false);

    // Receive and store status register value directly in 'status'
    if (!spi_transmitReceive(FRAM_SPI, NULL, &status, 1, false)) {
        spi_stopCommunication(FRAM_SPI_CS);
        return 0;  // Return 0 if read fails
    }
    // End communication
    spi_stopCommunication(FRAM_SPI_CS);
    return status;  // Return the read status register value
}

bool FRAM_readData(uint16_t address, uint8_t *buffer)
{
    // Validate address
    if (address >= FRAM_MAX_BYTES) {
        return false;
    }
    uint8_t command = FRAM_READ; 
    uint8_t addresses[2];

    addresses[0] = (address >> 16) & 0xFF; 
    addresses[1] = (address >> 8) & 0xFF;  

    spi_startCommunication(FRAM_SPI_CS);
    // Send FRAM_READ opcode
    if (!spi_transmitReceive(FRAM_SPI, &command, NULL, 1, false)) {
        spi_stopCommunication(FRAM_SPI_CS);
        return false;
    }
    // Send address bytes
    if (!spi_transmitReceive(FRAM_SPI, address, NULL, 2, false)) {
        spi_stopCommunication(FRAM_SPI_CS);
        return false;
    }
    // Read into buffer
    if (!spi_transmitReceive(FRAM_SPI, NULL, buffer, 256, false)) {
        spi_stopCommunication(FRAM_SPI_CS);
        return false;
    }
    spi_stopCommunication(FRAM_SPI_CS);
    return true;

    /*
    bool FRAM_readData(uint16_t address, uint8_t *buffer) {
    if (address >= FRAM_MAX_BYTES) { // Validate address (0x0000-0x7FFF)
        return false;
    }

    uint8_t command = FRAM_READ;
    uint8_t addresses[2] = {
        (address >> 8) & 0x7F, // High byte (mask bit 15)
        address & 0xFF          // Low byte
    };

    spi_startCommunication(FRAM_SPI_CS);

    // Send READ command
    if (!spi_transmitReceive(FRAM_SPI, &command, NULL, 1, false)) {
        spi_stopCommunication(FRAM_SPI_CS);
        return false;
    }

    // Send address bytes
    if (!spi_transmitReceive(FRAM_SPI, addresses, NULL, 2, false)) {
        spi_stopCommunication(FRAM_SPI_CS);
        return false;
    }

    // Read 256 bytes
    if (!spi_transmitReceive(FRAM_SPI, NULL, buffer, 256, false)) {
        spi_stopCommunication(FRAM_SPI_CS);
        return false;
    }

    spi_stopCommunication(FRAM_SPI_CS);
    return true;
    */
}

bool FRAM_writeData(uint16_t address, uint8_t* data, uint16_t size) {
    // Validate address
    if (address + size > FRAM_MAX_BYTES) {
        return false;
    }
    // Enable write operations
    if (!FRAM_writeEnable()) {
        return false;
    }

    uint8_t MOSI[3];
    MOSI[0] = FRAM_WRITE;             // Write command
    MOSI[1] = (address >> 8) & 0xFF;  // High byte of address
    MOSI[2] = address & 0xFF;         // Low byte of address
    spi_startCommunication(FRAM_SPI_CS);

    // Send the WRITE command and address
    if (!spi_transmitReceive(FRAM_SPI, &MOSI, NULL, 3, false)) {
        spi_stopCommunication(FRAM_SPI_CS);
        return false;
    }
    // Send data to write
    if (!spi_transmitReceive(FRAM_SPI, data, NULL, size, false)) {
        spi_stopCommunication(FRAM_SPI_CS);
        return false;
    }
    FRAM_wait();
    // End SPI communication
    spi_stopCommunication(FRAM_SPI_CS);

    return true;
}

static uint16_t pageToAddress(uint16_t page)
{
    return page * 256;
}

static uint16_t sectorToAddress(uint16_t sector)
{
    return sector * 4096;
}

bool FRAM_readPage(uint16_t page, uint8_t *buffer)
{
    uint16_t address = pageToAddress(page);
    return FRAM_readData(address, buffer);
}

bool FRAM_readSector(uint16_t sector, uint8_t *buffer)
{
    uint16_t address = sectorToAddress(sector);
    return FRAM_readData(address, buffer);
}

bool FRAM_writePage(uint16_t page, const uint8_t *data)
{
    uint16_t address = pageToAddress(page);
    return FRAM_writeData(address, data, 256);
}

bool FRAM_writeSector(uint16_t sector, const uint8_t *data)
{
    uint16_t address = sectorToAddress(sector);
    return FRAM_writeData(address, data, 4096);
}