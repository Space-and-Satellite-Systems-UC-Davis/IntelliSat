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

void fram_readDeviceID() {
    uint8_t MOSI = FRAM_READ_ID;
    uint8_t MISO[4];

    spi_startCommunication(FRAM_SPI_CS);
    spi_transmitReceive(FRAM_SPI, &MOSI, NULL, 1, false);
    spi_transmitReceive(FRAM_SPI, NULL, MISO, 4, false);
    spi_stopCommunication(FRAM_SPI_CS);

    printMsg("byte 1: %c\n", MISO[0]);
    printMsg("byte 2: %c\n", MISO[1]);
    printMsg("byte 3: %c\n", MISO[2]);
    printMsg("byte 4: %c\n", MISO[3]);

    /*
    printMsg("Manufacturer ID:\n");
    printBinary(MISO[0]);

    printMsg("\nContinuation Code:\n");
    printBinary(MISO[1]);

    printMsg("\nProduct ID:\n");
    printBinary(MISO[2]);
    printBinary(MISO[3]);
    */

}
