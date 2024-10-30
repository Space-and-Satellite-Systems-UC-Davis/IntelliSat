/*
 * MB85RS256B.c
 *
 *  Created on: Sep 3, 2024
 *      Author: green
 */

#include "MB85RS256B.h"

void printBinary(uint8_t byte) {
	for (int i = 7; i >= 0; i--) {
		printMsg("%c", (byte & (1 << i)) ? '1' : '0');
	}
	printMsg("\n\r");
}

void FRAM_read_deviceID(uint8_t* MISO) {
    uint8_t MOSI = FRAM_READ_ID;

    spi_startCommunication(FRAM_SPI_CS);
    spi_transmitReceive(FRAM_SPI, &MOSI, NULL, 1, false);
    spi_transmitReceive(FRAM_SPI, NULL, MISO, 5, false);
    spi_stopCommunication(FRAM_SPI_CS);
}
