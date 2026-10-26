/*
 * MB85RS256B.c
 *
 *  Created on: Sep 3, 2024
 *      Author: green
 * 
 *  Adapted to OPR3: 17 September 2026
 *      Author: 
 */

#include "MB85RS256B.h"
#include <stddef.h>

/* spi_transmitReceive already checks final BSY with a bounded timeout.
 * Do not add an unbounded wait after either success or failure. */

void FRAM_read_deviceID(uint8_t *MISO)
{
    uint8_t MOSI = FRAM_READ_ID;

    spi_startCommunication(FRAM_SPI_CS);
    spi_transmitReceive(FRAM_SPI, &MOSI, NULL, 1, false);
    spi_transmitReceive(FRAM_SPI, NULL, MISO, 4, false);
    spi_stopCommunication(FRAM_SPI_CS);
}

// WARNING: It appears writeEnable is necessary per write operation
bool FRAM_writeEnable() {
    uint8_t MOSI = FRAM_WRITE_ENABLE;

    // Activate SPI communication
    spi_startCommunication(FRAM_SPI_CS);

    // Send WRITE_ENABLE command
    if (!spi_transmitReceive(FRAM_SPI, &MOSI, NULL, 1, false)) {
        spi_stopCommunication(FRAM_SPI_CS);
        return false;
    }
    // End communication
    spi_stopCommunication(FRAM_SPI_CS);

    // Confirm WEL is set
    uint8_t status = FRAM_readStatusRegister();
    return (status & 0x02) != 0;  // Check if WEL bit is set
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

    // Activate SPI communication
    spi_startCommunication(FRAM_SPI_CS);

    // Send READ_STATUS_REGISTER command
    if (!spi_transmitReceive(FRAM_SPI, &MOSI, NULL, 1, false)) {
        spi_stopCommunication(FRAM_SPI_CS);
        return 0;  // Return 0 if command fails
    }

    // RDSR returns status immediately after the opcode; no dummy byte.
    // Receive and store status register value directly in 'status'
    if (!spi_transmitReceive(FRAM_SPI, NULL, &status, 1, false)) {
        spi_stopCommunication(FRAM_SPI_CS);
        return 0;  // Return 0 if read fails
    }
    // End communication
    spi_stopCommunication(FRAM_SPI_CS);
    return status;  // Return the read status register value
}

bool FRAM_readData(uint16_t address, uint8_t *buffer, uint16_t size)
{
    // Validate address
    if (buffer == NULL || size == 0 || address >= FRAM_MAX_BYTES
        || size > FRAM_MAX_BYTES - address) {
        return false;
    }
    uint8_t command = FRAM_READ; 
    uint8_t addresses[2] = {
        (address >> 8) & 0x7F, // Mask MSB (bit 15)
        address & 0xFF         // Low byte
    };

    spi_startCommunication(FRAM_SPI_CS);
    // Send FRAM_READ opcode
    if (!spi_transmitReceive(FRAM_SPI, &command, NULL, 1, false)) {
        spi_stopCommunication(FRAM_SPI_CS);
        return false;
    }
    // Send address bytes
    if (!spi_transmitReceive(FRAM_SPI, addresses, NULL, 2, false)) {
        spi_stopCommunication(FRAM_SPI_CS);
        return false;
    }

    // // Dummy read to handle timing/delay issues
    // uint8_t trash;
    // spi_transmitReceive(FRAM_SPI, NULL, &trash, 1, false);

    // Read into buffer
    if (!spi_transmitReceive(FRAM_SPI, NULL, buffer, size, false)) {
        spi_stopCommunication(FRAM_SPI_CS);
        return false;
    }
    spi_stopCommunication(FRAM_SPI_CS);
    return true;
}

bool FRAM_writeData(uint16_t address, uint8_t* data, uint16_t size) {
    // Validate address
    if (data == NULL || size == 0 || address >= FRAM_MAX_BYTES || size > FRAM_MAX_BYTES - address) {
        return false;
    }

    if ((FRAM_readStatusRegister() & 0x02) == 0) return false;

    uint8_t MOSI[3] = {
        FRAM_WRITE,
        (address >> 8) & 0x7F, // Mask MSB (bit 15)
        address & 0xFF
    };

    spi_startCommunication(FRAM_SPI_CS);

    // Send the WRITE command and address
    if (!spi_transmitReceive(FRAM_SPI, MOSI, NULL, 3, false)) {
        spi_stopCommunication(FRAM_SPI_CS);
        return false;
    }
    // Send data to write
    if (!spi_transmitReceive(FRAM_SPI, data, NULL, size, false)) {
        spi_stopCommunication(FRAM_SPI_CS);
        return false;
    }
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
    if (page >= FRAM_MAX_BYTES / 256U) return false;
    uint16_t address = pageToAddress(page);
    return FRAM_readData(address, buffer, 256);
}

bool FRAM_readSector(uint16_t sector, uint8_t *buffer)
{
    if (sector >= FRAM_MAX_BYTES / 4096U) return false;
    uint16_t address = sectorToAddress(sector);
    return FRAM_readData(address, buffer, 4096);
}

bool FRAM_writePage(uint16_t page, const uint8_t *data)
{
    if (page >= FRAM_MAX_BYTES / 256U) return false;
    uint16_t address = pageToAddress(page);
    /* The legacy SPI transmit API is mutable but does not modify its buffer. */
    return FRAM_writeData(address, (uint8_t *)data, 256);
}

bool FRAM_writeSector(uint16_t sector, const uint8_t *data)
{
    if (sector >= FRAM_MAX_BYTES / 4096U) return false;
    uint16_t address = sectorToAddress(sector);
    return FRAM_writeData(address, (uint8_t *)data, 4096);
}
