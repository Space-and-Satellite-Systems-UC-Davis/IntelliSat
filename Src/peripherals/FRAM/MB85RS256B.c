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

bool FRAM_readData(uint32_t address, uint8_t* buffer)
{
    uint8_t command[4];
    command[0] = FRAM_READ;                 // Read opcode
    command[1] = (address >> 16) & 0xFF;   // Address high byte
    command[2] = (address >> 8) & 0xFF;    // Address middle byte
    command[3] = address & 0xFF;           // Address low byte

    // Start SPI communication
    spi_startCommunication(FRAM_SPI_CS);

    // Send the read command with address
    if (!spi_transmitReceive(FRAM_SPI, command, NULL, sizeof(command), false))
    {
        spi_stopCommunication(FRAM_SPI_CS);
        return false;  // Command transmission failed
    }

    // Receive 256 bytes of data
    if (!spi_transmitReceive(FRAM_SPI, NULL, buffer, 256, false))
    {
        spi_stopCommunication(FRAM_SPI_CS);
        return false;  // Data reception failed
    }

    // Stop SPI communication
    spi_stopCommunication(FRAM_SPI_CS);

    return true;  // Operation successful
}