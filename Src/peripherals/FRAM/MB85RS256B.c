/*
 * MB85RS256B.c
 *
 *  Created on: Sep 3, 2024
 *      Author: green
 */

#include "MB85RS256B.h"

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

    uint8_t command = FRAM_READ; // Read opcode
    uint8_t addresses[2];

    addresses[1] = (address >> 16) & 0xFF; // Address high byte
    addresses[2] = (address >> 8) & 0xFF;  // Address middle byte

    spi_startCommunication(FRAM_SPI_CS);

    if (address > FRAM_MAX_BYTES)
    {
        // checks if the address is greater than 32768 bytes (8000 in hex).
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
