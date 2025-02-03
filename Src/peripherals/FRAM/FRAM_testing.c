/*
 * FRAM_testing.c
 *
 *  Created on: Sep 3, 2024
 *      Author: green
 *
 *       This document is testing and documentation for the MB85RS256B FRAM memory interface.
 */

#include "MB85RS256B.h"
#include "print_scan.h"

#define FRAM_NUM_TESTS 9 // Update as you add more tests
#define FRAM_TEST_PAGE 2
#define FRAM_TEST_ADDRESS 0x0000
#define FRAM_BUFFER_SIZE 256

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
/*                            HELPER FUNCTIONS                               */
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

// Print a byte in binary. Useful for visualizing status registers.
void printBinary(uint8_t byte)
{
    for (int i = 7; i >= 0; i--)
    {
        printMsg("%c", (byte & (1 << i)) ? '1' : '0');
    }
    printMsg("\n\r");
}

// Fill an array with a repeating value.
void fillBuf(uint8_t buf[], uint16_t size, uint8_t value) {
    for (uint16_t i = 0; i < size; i++) {
        buf[i] = value;
    }
}

// Clear an array by filling it with 0xFF.
void clearBuf(uint8_t buf[], uint16_t size) {
    for (uint16_t i = 0; i < size; i++) {
        buf[i] = 0xFF;
    }
}

// Print the contents of a buffer in hexadecimal, 16 bytes per line.
void printBuf(uint8_t buf[], uint16_t size) {
    for (uint16_t i = 0; i < size; i++) {
        printMsg("0x%02X ", buf[i]);
        if ((i + 1) % 16 == 0) {
            printMsg("\n\r");
        }
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
/*                              FUNCTION TESTS                               */
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

// FRAM: Basic communications
// Currently, bytes 1~3 are getting the correct 0x04 0x7F 0x05 code. byte 0 seems to get gibberish
// Tried byte 5. Looks like that picks up the 0x09. Need to figure out why byte 0 is wasted currently
// but otherwise, GOOD!
bool FRAMtest_read_deviceID()
{
	uint8_t MOSI[5];
	FRAM_read_deviceID(MOSI);
	for (uint8_t i = 0; i < 5; ++i)
	{
		printMsg("\n%u", MOSI[i]);
	}
	return true;
}
/*  Check whether Initial WEL bit is 0. Then see if write_enable passes or not.
    * expected: 
    * actual: 
*/
bool FRAMtest_writeEnable()
{
    // Read initial SR WEL bit
    uint8_t initialWEL = (FRAM_readStatusRegister() >> 1) & 0x01;
    FRAM_writeEnable();
    uint8_t updatedWEL = (FRAM_readStatusRegister() >> 1) & 0x01;

    // Test if the WEL bit is set
    if (updatedWEL == 1 && initialWEL == 0) {
        printMsg("Write Enable Test PASS\n\r");
        return true;
    }
	printMsg("Write Enable Test FAIL - WEL bit incorrect\n\r");
	return false;
}
/*  Reads current Status Register
    * expect: "Status Register Read Successful. Value: xyz"
    * actual: 
*/
bool FRAMtest_readStatusRegister() {
    printMsg("Starting Read Status Register Test...\n\r");

    // Read the status register
    uint8_t status = FRAM_readStatusRegister();

    // Check if the status register read was successful
    if (status != 0) {
    	printMsg("Status Register Read PASS. Value: 0x%02X\n\r", status);
    	return true;
    }
    printMsg("Read Status Register FAIL. Received 0x00 (possible error).\n\r");
    return false;
}
/*  Displays the FRAM data starting at 0x0 in hexadecimal format, 16 bytes per line.
    * expect: "FRAM Data Read Successful"
    * actual: 
*/
bool FRAMtest_readData()
{
	uint8_t buffer[256] = {0};
	uint16_t address = 0x000000;

	if (FRAM_readData(address, buffer)) {

		printMsg("FRAM Data Read PASS:\n\r");
		for (uint16_t i = 0; i < 256; ++i)
		{
			printMsg("0x%02X ", buffer[i]);
			if ((i + 1) % 16 == 0)
				printMsg("\n\r");
		}
		return true;
	}
	printMsg("FRAM Data Read FAIL.\n\r");
	return false;
}
/*  Sets page number to 2 and reads. Corresponds to the address range 0x0200 to 0x02FF.
    * expected: 
    * actual: 
*/
bool FRAMtest_readPage()
{
	uint8_t buffer[256] = {0};
	uint16_t page = 2;

	if (FRAM_readPage(page, buffer)) {
		printMsg("FRAM Page Read PASS (Page %u):\n\r", page);
		for (uint16_t i = 0; i < 256; ++i)
		{
			printMsg("0x%02X", buffer[i]);
			if ((i + 1) % 16 == 0)
				printMsg("\n\r");
		}
		return true;
	}
	printMsg("FRAM Page Read FAIL.\n\r");
	return false;
}

/* Write Data Tests (4 Tests)
  Test 1: Write to buffer with bits 1-256 and read.
  * expected: "Basic Write and Read Test Passed"
  * actual: 
  Test 2: Write to boundary of memory
  * expected: "Boundary Condition Test Passed"
  * actual: 
  Test 3: Write to Invalid Address (Expect rejection)
  * expected: "Invalid Address Test Passed (Command Rejected as Expected)."
  * actual: 
  Test 4: Write Protect Test
  * expected: "Write Protect Test Passed (Write Rejected as Expected)."
  * actual: 
*/

uint8_t writeBuffer[256] = {0};
uint8_t readBuffer[256] = {0};
uint32_t address = 0x0000;

bool FRAMtest_basicWriteData() {

    // Fill the buffer with a recognizable pattern
    for (uint16_t i = 0; i < 256; ++i) {
        writeBuffer[i] = i;
    }

    // Test 1: Basic Write and Read
    printMsg("Starting Basic Write and Read Test...\n\r");
    if (FRAM_writeData(address, writeBuffer, 256)) {
        FRAM_readData(address, readBuffer);

        // Verify written and read data
        for (uint16_t i = 0; i < 256; i++) {
            if (writeBuffer[i] != readBuffer[i]) {
                printMsg("Basic Write and Read Test Data mismatch at index %d: Sent %02X, Read %02X\n\r", i, writeBuffer[i], readBuffer[i]);
                return false;
            }
        }
        printMsg("Basic Write and Read Test PASS.\n\r");
        return true;
    }
    printMsg("Write Command FAIL.\n\r");
    return false;
}

bool FRAMtest_BoundaryWriteData() {
    // Test 2: Boundary Condition Write
    address = 0x7F00; // Near upper boundary of memory
    printMsg("Starting Boundary Condition Test at 0x7F00...\n\r");
    if (FRAM_writeData(address, writeBuffer, 256)) {

    	FRAM_readData(address, readBuffer);
        for (uint16_t i = 0; i < 256; i++) {
            if (writeBuffer[i] != readBuffer[i]) {
                printMsg("Boundary Condition Test Data Mismatch at index %d: Sent %02X, Read %02X\n\r", i, writeBuffer[i], readBuffer[i]);
                return false;
            }
        }
        printMsg("Boundary Condition Test PASS.\n\r");
        return true;
    }
    printMsg("Boundary Write Command FAIL.\n\r");
    return false;
}

bool FRAMtest_InvalidAddressWriteData() {
    // Test 3: Invalid Address Write (Expect Failure) (0x0000 - 0x7FFF valid range)
    address = 0x8000; // Invalid address
    printMsg("Starting Invalid Address Test at 0x8000...\n\r");
    if (!FRAM_writeData(address, writeBuffer, 256)) {
        printMsg("Invalid Address Test Passed (Command Rejected as Expected).\n\r");
        return true;
    }
    printMsg("Invalid Address Test Failed (Command Unexpectedly Accepted).\n\r");
    return false;
}

bool FRAMtest_WriteProtect() {
    // Test 4: Write Protect Test
    FRAM_writeEnable(); // Ensure WEL is set first
    FRAM_writeDisable(); // Clear WEL
    
    uint8_t status = FRAM_readStatusRegister();
    if ((status & 0x02) != 0) { // Check WEL is 0
        printMsg("Write Protect Test FAIL: WEL still set\n\r");
        return false;
    }
    // Try to write data (should fail)
    uint8_t dummyData[1] = {0xAA};
    if (!FRAM_writeData(0x0000, dummyData, 1)) {
        printMsg("Write Protect Test PASS\n\r");
        return true;
    }
    printMsg("Write Protect Test FAIL\n\r");
    return false;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
/*                            testFunction Core                              */
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

void testFunction_FRAM() {
    bool (*testFunctions[FRAM_NUM_TESTS])() = {
    	FRAMtest_read_deviceID,
		FRAMtest_writeEnable,
		FRAMtest_readStatusRegister,
		FRAMtest_readData,
		FRAMtest_readPage,
		FRAMtest_basicWriteData,
		FRAMtest_BoundaryWriteData,
		FRAMtest_InvalidAddressWriteData,
		FRAMtest_WriteProtect,
    };

    const char *testNames[FRAM_NUM_TESTS] = {
        "Read Device ID",
        "Write Enable",
        "Read Status Register",
		"Read Data",
		"Read Page",
        "Basic Write and Read",
		"Boundary Write",
        "Invalid Address Write",
        "Write Protect",
    };

    printMsg("\n\rFRAM Core Tests\n\r");

    for (uint8_t i = 0; i < FRAM_NUM_TESTS; i++) {
        printMsg("%s: %s\n\r", testNames[i], testFunctions[i]() ? "PASS" : "FAIL");
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
/*                               DEPRECATED                                  */
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
