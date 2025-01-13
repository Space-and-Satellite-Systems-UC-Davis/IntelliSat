/*
 * FRAM_testing.c
 *
 *  Created on: Sep 3, 2024
 *      Author: green
 */

#include "MB85RS256B.h"
#include "print_scan.h"

void FRAMtest_write_enable()
{

	uint8_t SR1 = FRAM_read_SR1();
	printMsg("\n\rSR1 initial: %c", SR1);

	FRAM_write_enable();

	SR1 = FRAM_read_SR1();
	printMsg("\n\rSR1 after WREn: %c", SR1);
}

// FRAM: Basic communications
// Currently, bytes 1~3 are getting the correct 0x04 0x7F 0x05 code. byte 0 seems to get gibberish
// Tried byte 5. Looks like that picks up the 0x09. Need to figure out why byte 0 is wasted currently
// but otherwise, GOOD!
void FRAMtest_read_deviceID()
{
	uint8_t MOSI[5];
	FRAM_read_deviceID(MOSI);
	for (uint8_t i = 0; i < 5; ++i)
	{
		printMsg("\n%u", MOSI[i]);
	}
}

void FRAMtest_readData()
{
	uint8_t buffer[256] = {0};
	uint16_t address = 0x000000;

	if (FRAM_readData(address, buffer))
	{

		printMsg("FRAM Data Read Successful:\n\r");
		for (uint16_t i = 0; i < 256; ++i)
		{
			printMsg("0x%02X ", buffer[i]);
			if ((i + 1) % 16 == 0)
				printMsg("\n\r");
		}
	}
	else
	{
		printMsg("FRAM Data Read Failed.\n\r");
	}
}

void FRAMtest_readPage()
{
	uint8_t buffer[256] = {0};
	uint16_t page = 2;

	if (FRAM_readPage(page, buffer))
	{
		printMsg("FRAM Page Read Successful (Page %u):\n\r", page);
		for (uint16_t i = 0; i < 256; ++i)
		{
			printMsg("0x%02X ", buffer[i]);
			if ((i + 1) % 16 == 0)
				printMsg("\n\r");
		}
	}
	else
	{
		printMsg("FRAM Page Read Failed.\n\r");
	}
}

void FRAMtest_writeData() {
    uint8_t writeBuffer[256] = {0};
    uint8_t readBuffer[256] = {0};
    uint32_t address = 0x0000;

    // Fill the buffer with a recognizable pattern
    for (uint16_t i = 0; i < 256; ++i) {
        writeBuffer[i] = i;
    }

    // Test 1: Basic Write and Read
    printMsg("Starting Basic Write and Read Test...\n\r");
    if (FRAM_writeData(address, writeBuffer, 256)) {
        printMsg("Write Command Sent.\n\r");
        FRAM_readData(address, readBuffer);

        // Verify written and read data
        for (uint16_t i = 0; i < 256; i++) {
            if (writeBuffer[i] != readBuffer[i]) {
                printMsg("Basic Write and Read Test Data mismatch\n\r", i, writeBuffer[i], readBuffer[i]); // at index %d: Sent %02X, Read %02X
                return;
            }
        }
        printMsg("Basic Write and Read Test Passed.\n\r");
    } else {
        printMsg("Write Command Failed.\n\r");
    }

    // Test 2: Boundary Condition Write
    address = 0x7F00; // Near upper boundary of memory
    printMsg("Starting Boundary Condition Test at 0x7F00...\n\r");
    if (FRAM_writeData(address, writeBuffer, 256)) {
        printMsg("Boundary Write Command Sent.\n\r");
        FRAM_readData(address, readBuffer);

        for (uint16_t i = 0; i < 256; i++) {
            if (writeBuffer[i] != readBuffer[i]) {
                printMsg("Boundary Condition Test Data Mismatch\n\r", i, writeBuffer[i], readBuffer[i]);
                return;
            }
        }
        printMsg("Boundary Condition Test Passed.\n\r");
    } else {
        printMsg("Boundary Write Command Failed.\n\r");
    }

    // Test 3: Invalid Address Write (Expect Failure) (0x0000 - 0x7FFF valid range)
    address = 0x8000; // Invalid address
    printMsg("Starting Invalid Address Test at 0x8000...\n\r");
    if (!FRAM_writeData(address, writeBuffer, 256)) {
        printMsg("Invalid Address Test Passed (Command Rejected as Expected).\n\r");
    } else {
        printMsg("Invalid Address Test Failed (Command Unexpectedly Accepted).\n\r");
    }

    // Test 4: Write Protect Test
    printMsg("Testing Write Protection...\n\r");
    if (FRAM_writeEnable()) {
        FRAM_writeDisable(); // Set WP
        if (!FRAM_writeData(0x0000, writeBuffer, 256)) {
            printMsg("Write Protect Test Passed (Write Rejected as Expected).\n\r");
        } else {
            printMsg("Write Protect Test Failed (Write Unexpectedly Allowed).\n\r");
        }
        FRAM_writeEnable(); // Reset WP
    } else {
        printMsg("Failed to Enable Write for Testing.\n\r");
    }

    printMsg("All Tests Completed.\n\r");

}

void testFunction_FRAM()
{
    FRAMtest_readData();
    FRAMtest_read_deviceID();
    FRAMtest_readPage();

}
