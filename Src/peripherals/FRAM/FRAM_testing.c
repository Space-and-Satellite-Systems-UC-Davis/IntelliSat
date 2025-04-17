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
 
 #define FRAM_NUM_TESTS 12 // Update as you add more tests
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
 
 // Print the contents of a buffer in hexadecimal, 16 bytes per line.
 void printBuf(uint8_t buf[], uint16_t size) {
     for (uint16_t i = 0; i < size; i++) {
         printMsg("0x%02X ", buf[i]);
         if ((i + 1) % 16 == 0) {
             printMsg("\n\r");
         }
     }
 }
 
 // Fill buffer
 void fillBuf(uint8_t buf[], uint16_t size, uint8_t value) {
     for (uint16_t i = 0; i < size; i++) {
         buf[i] = value;
     }
 }
 
 // Clear entire FRAM with 0xFF
 bool FRAM_clearAll() {
     uint8_t buffer[256];
     fillBuf(buffer, 256, 0xFF);  // Fill the buffer with 0xFF
 
     for (uint16_t page = 0; page < (FRAM_MAX_BYTES / 256); ++page) {
         if (!FRAM_writePage(page, buffer)) {
             printMsg("fatal: Failed to clear FRAM at page %u.\n\r", page);
             return false;
         }
     }
     printMsg("FRAM cleared.\n\r");
     return true;
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
         printMsg("%u ", MOSI[i]);
     }
     return true;
 }
 /*  Check whether Initial WEL bit is 0. Then see if write_enable passes or not.
     * expected: Write Enable: PASS
     * actual: fatal: WEL bit incorrect
 */
 bool FRAMtest_writeEnable()
 {
     // Read initial SR WEL bit
     uint8_t initialWEL = (FRAM_readStatusRegister() >> 1) & 0x01;
     FRAM_writeEnable();
     uint8_t updatedWEL = (FRAM_readStatusRegister() >> 1) & 0x01;
 
     // Test if the WEL bit is set
     if (updatedWEL == 1 && initialWEL == 0) {
         return true;
     }
     printMsg("fatal: WEL bit incorrect\n\r");
     return false;
 }
 /*  Reads current Status Register values
     * expect: "Read Status Register: PASS"
     * actual: "Read Status Register: PASS"
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
     printMsg("fatal: Read Status Register FAIL. Received 0x00 (possible error).\n\r");
     return false;
 }
 /*  Displays the FRAM data starting at 0x0 in hexadecimal format, 16 bytes per line. Should be all 0xFF.
     * expect: "Read Data: PASS"
     * actual: "Read Data: PASS"
 */
 bool FRAMtest_readData()
 {
     uint8_t buffer[256] = {0};
     uint16_t address = 0x000000;
     if (!FRAM_clearAll()) return false;
 
     if (FRAM_readData(address, buffer)) {
         printMsg("FRAM Data Read Starting...\n\r");
         printBuf(buffer, 256);
         return true;
     }
     printMsg("fatal: FRAM Read attempt failed\n\r");
     return false;
 }
 /*  Sets page number to 2 and reads. Corresponds to the address range 0x0200 to 0x02FF.
     * expected: "Read Page: PASS"
     * actual: "Read Page: PASS"
 */
 bool FRAMtest_readPage()
 {
     uint8_t buffer[256] = {0};
     uint16_t page = 2;
 
     if (FRAM_readPage(page, buffer)) {
         printMsg("FRAM Page Read (Page %u):\n\r", page);
         printBuf(buffer, 256);
         return true;
     }
     printMsg("fatal: FRAM Read Page Read attempt failed.\n\r");
     return false;
 }
 
 /*  Reads sector data (should initially be all 0xFF if cleared) */
 bool FRAMtest_readSector()
 {
     uint8_t buffer[4096] = {0};
     uint16_t sector = 1; // Read from sector 1 for testing
     if (!FRAM_clearAll()) return false;
 
     if (FRAM_readSector(sector, buffer)) {
         printMsg("FRAM Sector Read (Sector %u):\n\r", sector);
         printBuf(buffer, 256); // Print only the first 256 bytes for readability
         return true;
     }
     printMsg("fatal: FRAM Read Sector Read attempt failed.\n\r");
     return false;
 }
 
 /* Write Data Tests (4 Tests)
   Test 1: Write to buffer with bits 1-256 and read.
   * expected: "Basic Write and Read: PASS"
   * actual: "Basic Write and Read: PASS"
   Test 2: Write to boundary of memory
   * expected: "Boundary Write: PASS"
   * actual: "Boundary Write: PASS"
   Test 3: Write to Invalid Address (Expect rejection)
   * expected: "Invalid Address Write: PASS"
   * actual: "Invalid Address Write: PASS"
   Test 4: Write Protect Test
   * expected: "Write Protect: PASS"
   * actual: "fatal: Write Protect: Write succeeded despite WEL=0"
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
                 printMsg("fatal: Basic Write and Read Test Data mismatch at index %d: Sent %02X, Read %02X\n\r", i, writeBuffer[i], readBuffer[i]);
                 return false;
             }
         }
         return true;
     }
     printMsg("fatal: Write Command FAIL.\n\r");
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
                 printMsg("fatal: Boundary Condition Test Data Mismatch at index %d: Sent %02X, Read %02X\n\r", i, writeBuffer[i], readBuffer[i]);
                 return false;
             }
         }
         return true;
     }
     printMsg("fatal: Boundary Write Command FAIL.\n\r");
     return false;
 }
 
 bool FRAMtest_InvalidAddressWriteData() {
     // Test 3: Invalid Address Write (Expect Failure) (0x0000 - 0x7FFF valid range)
     address = 0x8000; // Invalid address
     printMsg("Starting Invalid Address Test at 0x8000...\n\r");
     if (!FRAM_writeData(address, writeBuffer, 256)) {
         printMsg("Invalid Address Test (Command Rejected as Expected).\n\r");
         return true;
     }
     printMsg("fatal: Invalid Address Test (Command Unexpectedly Accepted).\n\r");
     return false;
 }
 
 bool FRAMtest_WriteProtect() {
     // Ensure WEL is set first
     if (!FRAM_writeEnable()) {
         printMsg("fatal: Write Protect: Could not set WEL\n\r");
         return false;
     }
     // Clear WEL via WRDI
     if (!FRAM_writeDisable()) {
         printMsg("fatal: Write Protect: WRDI command failed\n\r");
         return false;
     }
     // Verify WEL is 0
     uint8_t status = FRAM_readStatusRegister();
     if ((status & 0x02) != 0) { 
         printMsg("fatal: Write Protect: WEL still set\n\r");
         return false;
     }
     // Attempt write (should fail)
     uint8_t dummy = 0xAA;
     if (FRAM_writeData(0x0000, &dummy, 1)) {
         printMsg("fatal: Write Protect: Write succeeded despite WEL=0\n\r");
         return false;
     }
     return true;
 }
 
 /*  Write a pattern to a page and verify it by reading back */
 bool FRAMtest_writePage()
 {
     uint8_t writeBuffer[256];
     uint8_t readBuffer[256] = {0};
     uint16_t page = 2;
 
     // Fill buffer with test data
     for (uint16_t i = 0; i < 256; i++) {
         writeBuffer[i] = i;  // Incremental pattern
     }
 
     printMsg("Starting Write Page Test at page %u...\n\r", page);
 
     if (!FRAM_writePage(page, writeBuffer)) {
         printMsg("fatal: Write to page %u failed.\n\r", page);
         return false;
     }
 
     printMsg("Write successful. Verifying data...\n\r");
 
     if (!FRAM_readPage(page, readBuffer)) {
         printMsg("fatal: Read from page %u failed.\n\r", page);
         return false;
     }
 
     for (uint16_t i = 0; i < 256; i++) {
         if (writeBuffer[i] != readBuffer[i]) {
             printMsg("fatal: Data mismatch at index %u: Expected 0x%02X, Read 0x%02X\n\r", i, writeBuffer[i], readBuffer[i]);
             return false;
         }
     }
 
     return true;
 }
 
 /*  Write a pattern to a sector and verify it by reading back */
 bool FRAMtest_writeSector()
 {
     uint8_t writeBuffer[4096];
     uint8_t readBuffer[4096] = {0};
     uint16_t sector = 1;
 
     // Fill buffer with test data
     for (uint16_t i = 0; i < 4096; i++) {
         writeBuffer[i] = (i % 256);  // Cyclic pattern
     }
 
     printMsg("Starting Write Sector Test at sector %u...\n\r", sector);
 
     if (!FRAM_writeSector(sector, writeBuffer)) {
         printMsg("fatal: Write to sector %u failed.\n\r", sector);
         return false;
     }
 
     printMsg("Write successful. Verifying data...\n\r");
 
     if (!FRAM_readSector(sector, readBuffer)) {
         printMsg("fatal: Read from sector %u failed.\n\r", sector);
         return false;
     }
 
     for (uint16_t i = 0; i < 4096; i++) {
         if (writeBuffer[i] != readBuffer[i]) {
             printMsg("fatal: Data mismatch at index %u: Expected 0x%02X, Read 0x%02X\n\r", i, writeBuffer[i], readBuffer[i]);
             return false;
         }
     }
 
     return true;
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
         FRAMtest_readSector,
         FRAMtest_basicWriteData,
         FRAMtest_BoundaryWriteData,
         FRAMtest_InvalidAddressWriteData,
         FRAMtest_WriteProtect,
         FRAMtest_writePage,
         FRAMtest_writeSector,
     };
 
     const char *testNames[FRAM_NUM_TESTS] = {
         "Read Device ID",
         "Write Enable",
         "Read Status Register",
         "Read Data",
         "Read Page",
         "Read Sector",
         "Basic Write and Read",
         "Boundary Write",
         "Invalid Address Write",
         "Write Protect",
         "Write Page",
         "Write Sector",
     };
 
     printMsg("\n\rFRAM Core Tests\n\r");
 
     for (uint8_t i = 0; i < FRAM_NUM_TESTS; i++) {
         printMsg("%s: %s\n\r", testNames[i], testFunctions[i]() ? "PASS" : "FAIL");
     }
 }
 
 //-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
 /*                               DEPRECATED                                  */
 //-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-