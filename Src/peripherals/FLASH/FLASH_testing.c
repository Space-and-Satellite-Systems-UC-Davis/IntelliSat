/*
 * FLASH_testing.c
 *
 * - July 29, 2024
 * - Author: Anthony Surkov
 *
 * - Last updated: 08-26-24
 *
 * This document is testing and documentation for the W25Q128JV FLASH memory interface.
 * testFunction_FLASH() is near the end.
 */

#include "FLASH/W25Q128JV.h"
#include <globals.h>
#include <print_scan.h>

// Defines for test parameters.
#define FLASH_NUM_TESTS     9
#define FLASH_TEST_PAGE     0
#define FLASH_TEST_SECTOR   0

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
/*                            HELPER FUNCTIONS                               */
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

// Print a byte in binary. Useful for visualizing status registers.
void printBinary(uint8_t byte) {
	for (int i = 7; i >= 0; i--) {
		printMsg("%c", (byte & (1 << i)) ? '1' : '0');
	}
	printMsg("\n\r");
}

// Print some string, sandwiched between line indents.
void printIndented(char* printme) {
	printMsg("\n\r");
	printMsg(printme);
	printMsg("\n\r");
}

// Print an array with spaces between elements, marking every 256 bytes (pages).
void printBuf(uint8_t buf[], int size) {
	printMsg("\n\r");
	for (int i = 0; i < size; i++) {
		if (i % 256 == 0) {
			printIndented("page break");
		}
		printMsg("%u", (unsigned int)buf[i]);
		printMsg(" ");
	}
	printMsg("\n\r");
}

// Fill an array with some integer x.
void fillBuf(uint8_t buf[], int size, int x) {
	for (int i = 0; i < size; i++) {
		buf[i] = x;
	}
}

// fillBuf only uses one integer, so it's hard to tell where pages break off.
// fillBuf_increment fills a sector-sized array with a page of 0's, 1's, 2's, ..., 15's.
void fillBuf_increment(uint8_t buf[]) {
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 256; j++) {
			buf[ (i * 256) + j] = i;
		}
	}
}

// clearBuf fills an array with FFh.
void clearBuf(uint8_t buf[], int size) {
	for (int i = 0; i < size; i++) {
		buf[i] = 0xFF;
	}
}

uint64_t getMean(uint64_t* data, uint16_t size) {
	uint64_t mean = 0;
	for (uint16_t i = 0; i < size; i++) {
		mean += data[i];
	}
	return (mean / size);
}

uint64_t getVariance(uint64_t* data, uint16_t size, uint64_t mean) {
    uint64_t variance = 0;
    for (uint16_t i = 0; i < size; i++) {
        uint64_t diff = data[i] - mean;
        variance += diff * diff;
    }
    return variance / size;
}

// Helper function for test_writeEnable()
void flash_readRegisterOne(uint8_t* register_one) {
	if (qspi_getStatus() == QSPI_BUSY) {
		printMsg("BUSY");
	}
	qspi_setCommand(
		QSPI_FMODE_INDIRECT_READ,
		QSPI_1_WIRE,
		QSPI_UNUSED,
		QSPI_UNUSED,
		0,
		QSPI_1_WIRE,
		false
  );
  qspi_sendCommand(
		0x05, //READ_REGISTER_ONE
		QSPI_UNUSED,
		1,
		register_one,
		QSPI_READ,
		QSPI_TIMEOUT_PERIOD
  );
}

// Helper function to sector tests. Fills a sector incrementally.
void fillSector(uint16_t sector) {
	uint32_t page = sector * FLASH_PAGES_PER_SECTOR;

	for (int i = 0; i < 16; i++) {
		uint8_t bufferWrite[FLASH_PAGE_SIZE];
		fillBuf(bufferWrite, FLASH_PAGE_SIZE, i);

		flash_writePage(page+i, bufferWrite);
	}
}

bool page_isClear(uint32_t page) {
  uint8_t pageBuffer[256];
  flash_readPage(page, pageBuffer);

  for (int i = 0; i < FLASH_PAGE_SIZE; i++) {
    if ( pageBuffer[i] != 0xFF ) {
      return false;
    }
  }
  return true;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
/*                              FUNCTION TESTS                               */
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

// DONE readJedecID(). Updated 08.31.24
// Goal: Basic verification of communication: retrieving manufacturer ID.
// * Expected: 00 00 00 -> EF 40 18
// * Actual: 00 00 00 -> EF 40 18
//
bool test_readJedecID() {
	uint8_t id_receiver[3];
	qspi_setCommand(
	  QSPI_FMODE_INDIRECT_READ, //read
	  QSPI_1_WIRE, //1 wire for instruction
	  QSPI_UNUSED, //no wire for address
	  QSPI_UNUSED, //no wire for alt bytes
	  QSPI_UNUSED, //0 dummy cycles for JEDEC
	  QSPI_1_WIRE, //1 wire for data
	  false //no DMA
	);
	qspi_sendCommand(
	  0x9F, //JEDEC ID
	  QSPI_UNUSED, //no address
	  3, //3 bytes data
	  id_receiver, //where data goes
	  QSPI_READ,
	  QSPI_TIMEOUT_PERIOD
	);

	if (id_receiver[0] != 0xEF) {
		printMsg("readJedecID: unexpected ID, expected 0xEF4018\n\r");
		return false;
	}
	if (id_receiver[1] != 0x40) {
		printMsg("readJedecID: unexpected ID, expected 0xEF4018\n\r");
		return false;
	}
	if (id_receiver[2] != 0x18) {
		printMsg("readJedecID: unexpected ID, expected 0xEF4018\n\r");
		return false;
	}
	return true;
}

// DONE readRegisterOne(), writeEnable(). Updated 08.31.24
// Goal: Enable write. Read status register one.
// * Expected: 00000000 -> 00000010
// * Actual: 00000000 -> 00000010
//
bool test_writeEnable() {
	uint8_t register_one = 0;

	flash_readRegisterOne(&register_one);
	if ((register_one >> 1) & 1) {
		printMsg("writeEnable: register already set to 1\n\r");
		return false;
	}

	flash_writeEnable();
	flash_readRegisterOne(&register_one);

	return (register_one == 0b00000010);
}

// DONE readRegisterTwo(), quadEnable(). Updated 08.31.2024
// Goal: Enable quadSPI. Read status register two.
// * Expected: 00000000 -> 00000010
// * Actual: 00000000 -> 00000010
//
bool test_quadEnable() {
	uint8_t register_two = 0;

	flash_readRegisterTwo(&register_two);
	if (!((register_two >> 1) & 1)) {
		printMsg("quadEnable: register already set to 1\n\r");
	}

	flash_quadEnable();
	flash_readRegisterTwo(&register_two);

	return (register_two == 0b00000010);
}

// DONE writePage(). Updated 08.31.24
// Goal: Simultaneous verification of writePage(), readPage(). Difficult to do separately.
// * Expected: First read, full read buffer of 0xFF (empty. Second read, full buffer of 5's.
// * Actual: Matches.
//
bool test_writePage() {
	uint32_t page = FLASH_TEST_PAGE;

	uint8_t bufferWrite[FLASH_PAGE_SIZE];
	uint8_t bufferRead[FLASH_PAGE_SIZE];

	fillBuf(bufferWrite, FLASH_PAGE_SIZE, 5); // 5 is arbitrary

	if ( !page_isClear(page) ) {
		printMsg("writePage: page is not clear, trying eraseSector\n\r");
		flash_eraseSector(page);
		if ( !page_isClear(page) ) {
			return false;
		}
		printMsg("writePage: eraseSector OK\n\r");
	}

	flash_writePage(page, bufferWrite);
	flash_readPage(page, bufferRead);

	for (uint16_t i = 0; i < FLASH_PAGE_SIZE; i++) {
		if ( bufferRead[i] != 5 ) {
			printMsg("fatal: unexpected value in page after flash write\n\r");
			return false;
		}
	}
	return true;
}

// DONE eraseSector(). Updated 09.01.24
// Goal: Read a page, erase the sector, write a page, read that page.
// * expected: All 3's on a freshly-written page, then all 256's (empty) after erase
// * actual: matches
//
bool test_eraseSector() {
	uint16_t sector = FLASH_TEST_SECTOR;
	uint32_t page = sector / 16;

	bool allPagesClear = true;
	for (int i = 0; i < 16; i++) {
		if ( !page_isClear(page+i) ) {
			allPagesClear = false;
			continue;
		}
	}

	uint8_t bufferRead[FLASH_PAGE_SIZE];

	if (allPagesClear == true) {
		//printMsg("eraseSector: all pages empty. Trying pageWrite...\n\r");

		uint8_t bufferWrite[FLASH_PAGE_SIZE];
		fillBuf(bufferWrite, FLASH_PAGE_SIZE, 3); //3 is arbitrary

		flash_writePage(page, bufferWrite);
		flash_readPage(page, bufferRead);

		if (page_isClear(page)) {
			printMsg("fatal: all pages empty & pageWrite failed\n\r");
			return false;
		}
		//printMsg("eraseSector: pageWrite OK\n\r");
	}

	flash_eraseSector(sector);

	if (page_isClear(page)) {
		return true;
	}
	printMsg("fatal: page is not clear after erasing sector\n\r");
	return false;
}

// DONE readSector(). Updated 09.01.24
// Goal: Fill a sector, read it with readSector() and incrementally with pageRead().
//       Compare each element of both. Print true if identical (more visible than returning it)
//
// * expected: "true" printed. (will print "false" on loop otherwise);
// * actual: printed "true"
//
bool test_readSector() {
	uint16_t sector = FLASH_TEST_SECTOR;
	uint32_t page = sector * FLASH_PAGES_PER_SECTOR;

	uint8_t sectorBuffer[FLASH_SECTOR_SIZE];
	uint8_t pageBuffer[FLASH_PAGE_SIZE];

	flash_eraseSector(sector);
	fillSector(sector);
	flash_readSector(sector, sectorBuffer);

	for (uint8_t i = 0; i < FLASH_PAGES_PER_SECTOR; i++) {
		flash_readPage(page+i, pageBuffer);
		for (uint16_t j = 0; j < FLASH_PAGE_SIZE; j++) {
			if ( pageBuffer[j] != sectorBuffer[( i * FLASH_PAGE_SIZE) + j] ) {
				printMsg("fatal: readSector output does not match readPage output\n\r");
				return false;
			}
		}
	}
	return true;
}

// DONE writeSector(). Updated 09.01.24
// Goal: Erase a sector, read it. Write a filled buffer to it, read it again.
//		 Validate that the first read is empty. Validate that the second read is all filled.
// * expected: First sector all 256's. Second sector all 6's.
// * actual: Matches. Also, looks perfect on print. See "optional validation" below.
//
bool test_writeSector() {
	uint16_t sector = FLASH_TEST_SECTOR;

	uint8_t bufferWrite[FLASH_SECTOR_SIZE];
	uint8_t bufferRead_A[FLASH_SECTOR_SIZE];
	uint8_t bufferRead_B[FLASH_SECTOR_SIZE];

	flash_eraseSector(sector);
	flash_readSector(sector, bufferRead_A);
	fillBuf_increment(bufferWrite);
	flash_writeSector(sector, bufferWrite);
	flash_readSector(sector, bufferRead_B);

	for (uint32_t i = 0; i < FLASH_SECTOR_SIZE; i++) {
		if (bufferRead_A[i] != 255) { //0xFF
			printMsg("fatal: eraseSector failed. Initial sector not empty\n\r");
			return false;
		}
		if (bufferRead_B[i] != (i / 256) ) { //value written by fillBuf_increment
			printMsg("fatal: written page does not match writeSector output\n\r");
			return false;
		}
  	}
	return true;
}

// DONE readCustom(). Updated 09.01.24
// Goal: Erase a sector, write to it, read some random amount of bytes from it.
//       Compare output to readSector(). Return true if all elements match.
// * expected: true
// * actual: true
//
bool test_readCustom() {
	uint16_t sector = FLASH_TEST_SECTOR;
	uint32_t page = sector / 16;

	uint8_t buffer_customSize[500]; //arbitrary size
	uint8_t buffer_sectorRead[FLASH_SECTOR_SIZE];
	uint8_t buffer_sectorWrite[FLASH_SECTOR_SIZE];
	fillBuf_increment(buffer_sectorWrite);

	flash_eraseSector(sector);
	flash_readSector(sector, buffer_sectorRead);

	for (int i = 0; i < 16; i++) {
		if ( !page_isClear(page) ) {
			printMsg("fatal: eraseSector failed. Initial sector not empty\n\r");
			return false;
		}
	}

	flash_writeSector(sector, buffer_sectorWrite);
	flash_readCustom(sector, buffer_customSize, 500);
	flash_readSector(sector, buffer_sectorRead);

	for (uint32_t i = 0; i < 500; i++) {
		if ( buffer_customSize[i] != buffer_sectorRead[i] ) {
			printMsg("fatal: custom read output does not match readSector output\n\r");
			return false;
		}
	}
	return true;
}

// DONE writeCustom(). Updated 09.01.24
// Goal: Erase a sector, write a variable amount of bytes to it. The buffer should have different
//       values across pages to check that elements are correctly preserved across page breaks.
//       A variety of buffer sizes should be tested.
// Expected: If size < 256, should just partially write one page (remainder is FFh).
//           If size > 256, should fill first page (4), then write second page with different values (5).
//           If buffer is larger than size to be written, should write buffer partially.
// Actual: All matches. Buffers of size 5 and 260 used. Buffer of size 1000, with 512 bytes written, used.
//

//dev note: replace architecture to not need size, sector fxn arguments
bool test_writeCustom() {
	uint16_t sector = FLASH_TEST_SECTOR;
	uint32_t page = sector / 16;

	uint8_t buffer_customWrite[500]; //arbitrary size
	uint8_t buffer_sectorRead[FLASH_SECTOR_SIZE];
	fillBuf(buffer_customWrite, 500, 2); //2 is arbitrary

	flash_eraseSector(sector);
	flash_readSector(sector, buffer_sectorRead);

	for (uint8_t i = 0; i < 16; i++) {
		if ( !page_isClear(page) ) {
			printMsg("fatal: eraseSector failed. Initial sector not empty\n\r");
			return false;
		}
	}

	flash_writeCustom(page, buffer_customWrite, 500);
	flash_readSector(sector, buffer_sectorRead);

	for (uint32_t i = 0; i < FLASH_SECTOR_SIZE; i++) {
		if (i < 500) {
			if (buffer_sectorRead[i] != buffer_customWrite[i]) {
				printMsg("fatal: custom write page does not match sector read output");
				return false;
			}
		}
		if (i >= 500) {
			if (buffer_sectorRead[i] != 0xFF) {
				printMsg("unexpected: unwritten remainder of sector returned nonzero value");
				return false;
			}
		}
	}
	return true;
}

// Note (08.04.2024): flash_wait() does not really need its own testing function. We know it
// works by judging the behavior of functions that rely on it. Prior to 07.28.2024, all
// functions were broken due to a mistaken reliance on qspi_wait() for timing; flash commands
// would overlap and return nonsense, since functions did not wait for system readiness before
// trying to read/write more. After flash_wait() was successfully implemented, successive reads
// and writes started working. So, flash_wait() must work.


//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
/*                              TIMING TESTS                                 */
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

// Timing tests were complete by either tracking getSysTime() for larger functions, or by manually
// recording values from Logic2 software. In the latter case, this was done to achieve higher
// fidelity where necessary, since getSysTime() only has millisecond precision, and some commands are
// complete in ~2 ms. Raw data is available as Sheets documentation.
// Note that flash_wait(), writeEnable() are included in these timings.
// Means and standard deviations were calculated for each data set.


// DONE flash_readPage - 15 data points manually recorded. 08.08.2024
// MEAN: 2.1041 ms
// STDEV: 0.00078 ms
//
void time_readPage() {
	uint8_t readBuffer[FLASH_PAGE_SIZE];

	printMsg("start");

	for (int i = 0; i < 15; i++) {
		flash_readPage(i, readBuffer);
		printMsg(" ");
	}
}


// DONE flash_writePage() - 15 data points manually recorded. 08.07.2024
// MEAN:  2.2091 ms
// STDEV: 0.00064 ms
//
void time_writePage() {
	uint8_t writeBuffer[FLASH_PAGE_SIZE];
	fillBuf(writeBuffer, FLASH_PAGE_SIZE, 1);

	flash_eraseSector(20); flash_eraseSector(21);
	printMsg("start"); //asynch serial helps demarcate commands

	for (int i = 0; i < 15; i++) {
		flash_writePage(i, writeBuffer);
		printMsg(" ");
	}
}


// DONE flash_readSector - 30 data points. 08.09.2024
// MEAN: 32 ms
// STDEV: 0 ms
//
void time_readSector() {
	uint8_t readBuffer[FLASH_SECTOR_SIZE];
	uint64_t netTimes[30];

	for (uint8_t i = 0; i < 30; i++) {
		uint64_t timeOne = getSysTime();
		flash_readSector(i, readBuffer);
		uint64_t timeTwo = getSysTime();
		netTimes[i] = timeTwo - timeOne;
	}

	uint64_t mean = getMean(netTimes, 30);
	uint64_t variance = getVariance(netTimes, 30, mean);

	printMsg("%lu\n", (unsigned long)mean);
	printMsg("%lu\n", (unsigned long)variance);
}

// DONE flash_writeSector - 30 data points. 08.09.2024
// MEAN: 37 ms
// STDEV: 1.73 ms (variance 3 ms)
//
void time_writeSector() {
	uint8_t writeBuffer[FLASH_SECTOR_SIZE];
	uint64_t netTimes[30];
	fillBuf(writeBuffer, FLASH_SECTOR_SIZE, 1);
	flash_eraseSector(0); flash_eraseSector(1);

	for (uint8_t i = 0; i < 30; i++) {
		uint64_t timeOne = getSysTime();
		flash_writeSector(i, writeBuffer);
		uint64_t timeTwo = getSysTime();
		netTimes[i] = timeTwo - timeOne;
	}

	uint64_t mean = getMean(netTimes, 30);
	uint64_t variance = getVariance(netTimes, 30, mean);

	printMsg("%lu\n", (unsigned long)mean);
	printMsg("%lu\n", (unsigned long)variance);
}


//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
/*                            testFunction Core                              */
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

void testFunction_FLASH() {

  bool (*testFunctions[FLASH_NUM_TESTS])() = { test_readJedecID, test_writeEnable,
		  	  	  	  	  	    	   	   	   test_quadEnable, test_eraseSector,
											   test_writePage, test_readSector,
											   test_writeSector, test_readCustom,
											   test_writeCustom };

  const char *testNames[FLASH_NUM_TESTS] = { "readJedecID()", "writeEnable()",
		  	  	  	  	  	  	  	   	     "quadEnable()", "eraseSector()",
											 "writePage()", "readSector()",
											 "writeSector()", "readCustom()",
											 "writeCustom()" };

  printMsg("\n\r");
  printIndented("FLASH Core Tests");
  printMsg("\n\r");

  for (uint8_t i = 0; i < FLASH_NUM_TESTS; i++) {
      printMsg("%s: %s\n\r", testNames[i], testFunctions[i]() ? "OK" : "FAIL");
  }

  return;
}


//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
/*                               DEPRECATED                                  */
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

// DONE fillSector(). Updated 08.01.24
// Goal: Validation of fillSector(). Deprecated. Not part of testFunction_FLASH().
// * expected: 16 pages printed; page 0: 00...0, page 1: 11...1, ..., page 15: 15 15... 15
// * actual: matches
//
void test_fillSector() {
	flash_eraseSector(0);
	fillSector(0);
	uint8_t bufferMiso[256];
	uint32_t page = 0;
	for (uint8_t i = 0; i < 16; i++) {
		flash_readPage(page+i, bufferMiso);
		printBuf(bufferMiso, 256);
	}
}

// See test_writeEnable(). Updated 09.01.24
bool test_readRegisterOne() {
	uint8_t register_one = 0;
	printIndented("Empty register one:");
	printBinary(register_one);

	flash_readRegisterOne(&register_one);

	printIndented("Full register one:");
	printBinary(register_one);

	return true;
}
