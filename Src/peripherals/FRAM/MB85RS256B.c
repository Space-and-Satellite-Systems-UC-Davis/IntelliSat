/*
 * MB85RS256B.c
 *
 *  Created on: Sep 3, 2024
 *      Author: green
 */

#include "MB85RS256B.h"
#include <stddef.h>

void printBinary(uint8_t byte)
{
    for (int i = 7; i >= 0; i--)
    {
        printMsg("%c", (byte & (1 << i)) ? '1' : '0');
    }
    printMsg("\n\r");
}

void FRAM_read_deviceID(uint8_t *MISO)
{
    uint8_t MOSI = FRAM_READ_ID;

    spi_startCommunication(FRAM_SPI_CS);
    spi_transmitReceive(FRAM_SPI, &MOSI, NULL, 1, false);
    spi_transmitReceive(FRAM_SPI, NULL, MISO, 5, false);
    spi_stopCommunication(FRAM_SPI_CS);
}

bool FRAM_readData(uint16_t address, uint8_t *buffer)
{

    uint8_t command = FRAM_READ; 
    uint8_t addresses[2];

    addresses[1] = (address >> 16) & 0xFF; 
    addresses[2] = (address >> 8) & 0xFF;  

    spi_startCommunication(FRAM_SPI_CS);

    if (address > FRAM_MAX_BYTES)
    {
        spi_transmitReceive(FRAM_SPI, &command, NULL, 1, false);
    }

    if (!spi_transmitReceive(FRAM_SPI, address, NULL, sizeof(command), false))
    {
        spi_stopCommunication(FRAM_SPI_CS);
        return false;
    }

    if (!spi_transmitReceive(FRAM_SPI, NULL, buffer, 256, false))
    {
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

bool FRAM_writeData(uint16_t address, const uint8_t *data, size_t length)
{
    uint8_t command = FRAM_WRITE; 
    uint8_t addresses[2];

    addresses[0] = (address >> 8) & 0xFF; 
    addresses[1] = address & 0xFF;        

    spi_startCommunication(FRAM_SPI_CS);

    if (!spi_transmitReceive(FRAM_SPI, &command, NULL, 1, false))
    {
        spi_stopCommunication(FRAM_SPI_CS);
        return false;
    }

    if (!spi_transmitReceive(FRAM_SPI, addresses, NULL, 2, false))
    {
        spi_stopCommunication(FRAM_SPI_CS);
        return false;
    }

    if (!spi_transmitReceive(FRAM_SPI, data, NULL, length, false))
    {
        spi_stopCommunication(FRAM_SPI_CS);
        return false;
    }

    spi_stopCommunication(FRAM_SPI_CS);

    return true;
}
