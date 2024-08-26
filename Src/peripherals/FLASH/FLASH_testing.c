/*
 * branch_main.c (FLASH testing)
 *
 * - July 29, 2024
 * - Author: Anthony Surkov
 *
 * - Last updated: 08-09-24
 *
 * This document is testing and documentation for the W25Q128JV FLASH memory interface.
 */

#include "FLASH/W25Q128JV.h"
#include "print_scan.h"

// ***************** General helper functions. *****************//

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


//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
/*                              FUNCTION TESTS                               */
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-


// DONE readJedecID(), rewritten 07.29.2024
// Goal: Basic verification of communication: retrieving manufacturer ID.
// * Expected: 00 00 00 -> EF 17 9F
// * Actual: 00 00 00 -> EF 17 9F
//
void test_readJedecID() {
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

  return (id_receiver == 0xEF4018);
}

// DONE writePage(), rewritten 07.29.2024
// Goal: Simultaneous verification of writePage(), readPage(). Difficult to do separately.
// * Expected: First read, full read buffer of 0xFF (empty. Second read, full buffer of 5's.
// * Actual: Matches.
//
void test_writePage(uint16_t page) {
	uint8_t bufferWrite[FLASH_PAGE_SIZE];
	uint8_t bufferRead_Before[FLASH_PAGE_SIZE];
	uint8_t bufferRead_After[FLASH_PAGE_SIZE];

	fillBuf(bufferWrite, FLASH_PAGE_SIZE, 5);

	flash_readPage(page, bufferRead_Before);
	flash_writePage(page, bufferWrite);
	flash_readPage(page, bufferRead_After);

	printIndented("before");
	printBuf(bufferRead_Before, FLASH_PAGE_SIZE);

	printIndented("after");
	printBuf(bufferRead_After, FLASH_PAGE_SIZE);
}

// Helper function for flash_writeEnable()
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

// See below
void test_readRegisterOne() {
	uint8_t register_one = 0;
	printIndented("Empty register one:");
	printBinary(register_one);

	flash_readRegisterOne(&register_one);
	printIndented("Full register one:");
	printBinary(register_one);
}

// DONE readRegisterOne(), writeEnable(). Rewritten 07.29.2024
// Goal: Enable write. Read status register one.
// * Expected: 00000000 -> 00000010
// * Actual: 00000000 -> 00000010
//
void test_writeEnable() {
	uint8_t register_one = 0;

    flash_readRegisterOne(&register_one);
    printIndented("Empty register one:");
    printBinary(register_one);

    flash_writeEnable();

    flash_readRegisterOne(&register_one);
    printIndented("Full register one:");
    printBinary(register_one);
}

// DONE readRegisterTwo(), quadEnable(), 08.04.2024
// Goal: Enable quadSPI. Read status register two.
// * Expected: 00000000 -> 00000010
// * Actual: 00000000 -> 00000010
//
void test_quadEnable() {
	uint8_t register_two = 0;

	printIndented("Empty register_two variable:");
	printBinary(register_two);

	flash_quadEnable();
	flash_readRegisterTwo(&register_two);

	printIndented("Read register two:");
	printBinary(register_two);
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

// DONE fillSector(), 08.01.2024
// Goal: Validation of fillSector().
// * expected: 16 pages printed; page 0: 00...0, page 1: 11...1, ..., page 15: 15 15... 15
// * actual: matches
//
void test_fillSector() {
	flash_eraseSector(0);
	fillSector(0);
	uint8_t bufferMiso[256];
	uint32_t page = 0;
	for (int i = 0; i < 16; i++) {
		flash_readPage(page+i, bufferMiso);
		printBuf(bufferMiso, 256);
	}
}

// DONE eraseSector() 08.01.2024
// Goal: Read a page, erase the sector, write a page, read that page.
// * expected: All 3's on a freshly-written page, then all 256's (empty) after erase
// * actual: matches
//
bool test_eraseSector(uint16_t sector) {

	uint8_t bufferWrite[FLASH_PAGE_SIZE];
	uint8_t bufferRead_A[FLASH_PAGE_SIZE];
	uint8_t bufferRead_B[FLASH_PAGE_SIZE];
	uint32_t page = sector * FLASH_PAGES_PER_SECTOR;

	fillBuf(bufferWrite, FLASH_PAGE_SIZE, 3);
	flash_writePage(page, bufferWrite);

	flash_readPage(page, bufferRead_A);
	flash_eraseSector(sector);
	flash_readPage(page, bufferRead_B);

	printIndented("Before erase:");
	printBuf(bufferRead_A, FLASH_PAGE_SIZE);

	printIndented("After erase:");
	printBuf(bufferRead_B, FLASH_PAGE_SIZE);

	return true;
}

// DONE readSector(), 08.02.2024
// Goal: Fill a sector, read it with readSector() and incrementally with pageRead().
//       Compare each element of both. Print true if identical (more visible than returning it)
//
// * expected: "true" printed. (will print "false" on loop otherwise);
// * actual: printed "true"
//
bool test_readSector(uint16_t sector) {
	flash_eraseSector(sector);
	fillSector(sector);

	uint8_t sectorBuffer[FLASH_SECTOR_SIZE];
	flash_readSector(sector, sectorBuffer);

	uint8_t pageBuffer[FLASH_PAGE_SIZE];
	uint32_t page = sector * FLASH_PAGES_PER_SECTOR;

	for (int i = 0; i < FLASH_PAGES_PER_SECTOR; i++) {
		flash_readPage(page+i, pageBuffer);
		for (int j = 0; j < FLASH_PAGE_SIZE; j++) {
			if ( pageBuffer[j] != sectorBuffer[( i * FLASH_PAGE_SIZE) + j] ) {
				printIndented("false");
				return false;
			}
		}
	}

	printIndented("true");
	return true;
}

// DONE writeSector(), 08.03.2024
// Goal: Erase a sector, read it. Write a filled buffer to it, read it again.
//		 Validate that the first read is empty. Validate that the second read is all filled.
// * expected: First sector all 256's. Second sector all 6's.
// * actual: Matches. Also, looks perfect on print. See "optional validation" below.
//
bool test_writeSector(uint32_t sector) {
	uint8_t bufferWrite[FLASH_SECTOR_SIZE];
	uint8_t bufferRead_A[FLASH_SECTOR_SIZE];
	uint8_t bufferRead_B[FLASH_SECTOR_SIZE];

	// Erase the sector, read it, write to it, read it again
	flash_eraseSector(sector);
	flash_readSector(sector, bufferRead_A);
	fillBuf_increment(bufferWrite); // See notes on this function in helper function section
	flash_writeSector(sector, bufferWrite);
	flash_readSector(sector, bufferRead_B);

	for (int i = 0; i < 4096; i++) {
	/* Optional verbose validation
	  	printMsg("%u", i);
	 	printMsg(" ");
	  	printMsg("%u", bufferRead_B[i]);
	  	printMsg(" ");
	  	printMsg("%u", bufferRead_A[i]);
	  	printMsg(" ");
	  	printMsg("\n\r");
	*/
		if (bufferRead_A[i] != 255) { //0xFF
			printIndented("false");
			printMsg("\n\r");
			return false;
		}
		if (bufferRead_B[i] != (i / 256) ) { //written value
			printIndented("false");
			printMsg("\n\r");
			return false;
		}
  	}
	printIndented("true");
	return true;
}

// DONE readCustom(), 08.03.2024
// Goal: Erase a sector, write to it, read some random amount of bytes from it.
//       Compare output to readSector(). Return true if all elements match.
// * expected: true
// * actual: true
//
bool test_readCustom(uint16_t size, uint16_t sector) {
	uint8_t buffer_randomSize[size];
	uint8_t buffer_sectorRead[4096];
	uint8_t buffer_sectorWrite[4096];
	fillBuf_increment(buffer_sectorWrite);

	flash_eraseSector(sector);
	flash_writeSector(sector, buffer_sectorWrite);
	flash_readCustom(sector, buffer_randomSize, size);
	flash_readSector(sector, buffer_sectorRead);

	for (int i = 0; i < size; i++) {
		if ( buffer_randomSize[i] != buffer_sectorRead[i] ) {
			printIndented("false");
			return false;
		}
	}
	return true;
}

// DONE writeCustom(), 08.04.2024
// Goal: Erase a sector, write a variable amount of bytes to it. The buffer should have different
//       values across pages to check that elements are correctly preserved across page breaks.
//       A variety of buffer sizes should be tested.
// Expected: If size < 256, should just partially write one page (remainder is FFh).
//           If size > 256, should fill first page (4), then write second page with different values (5).
//           If buffer is larger than size to be written, should write buffer partially.
// Actual: All matches. Buffers of size 5 and 260 used. Buffer of size 1000, with 512 bytes written, used.
//
bool test_writeCustom(uint32_t page, uint32_t size) {
	uint8_t bufferWrite_custom[size];
	uint8_t bufferRead_sector[4096];

	//custom fillBuf. I want to see if values in the tail are preserved
	for (uint32_t i = 0; i < size; i++) {
		if (i >= 256) {
			bufferWrite_custom[i] = 5;
		}
		else {
			bufferWrite_custom[i] = 4;
		}
	}
	printBuf(bufferWrite_custom, size);

	uint16_t sector = page / 16;
	flash_eraseSector(sector);
	flash_writeCustom(page, bufferWrite_custom, size);
	flash_readSector(sector, bufferRead_sector);

	printBuf(bufferRead_sector, 4096);

	return true;
}

// Note (08.04.2024): flash_wait() does not really need its own testing function. We know it works by
//       judging the behavior of functions that rely on it. Prior to 07.28.2024, all functions were
//       broken due to a mistaken reliance on qspi_wait() for timing; flash commands would overlap and
//       return nonsense, since functions did not wait for system readiness before trying to write/read
//       more. After flash_wait() was successfully implemented, successive reads and writes started
//       working. So, flash_wait() must work.


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
	uint8_t readBuffer[256];

	printMsg("start");

	for (int i = 0; i < 30; i++) {
		flash_readPage(i, readBuffer);
		printMsg(" ");
	}
}


// DONE flash_writePage() - 15 data points manually recorded. 08.07.2024
// MEAN:  2.2091 ms
// STDEV: 0.00064 ms
//
void time_writePage() {
	uint8_t writeBuffer[256];
	fillBuf(writeBuffer, 256, 1);

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
	uint8_t readBuffer[4096];
	uint64_t netTimes[30];

	for (int i = 0; i < 30; i++) {
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
	uint8_t writeBuffer[4096];
	uint64_t netTimes[30];
	fillBuf(writeBuffer, 4096, 1);
	flash_eraseSector(0); flash_eraseSector(1);

	for (int i = 0; i < 30; i++) {
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

void testFunction_FLASH() {
  const int num_tests = 10;
  bool test_results[num_tests];

  const char *test_names[] = { "Test: readJedecID()", "Test: writeEnable()",
                               "Test: readRegisterOne()", "Test: quadEnable()",
                               "Test: eraseSector()", "Test: writePage()",
                               "Test: readSector()", "Test: writeSector()",
                               "Test: readCustom()", "Test: writeCustom()" }

  test_results[0] = test_readJedecID();
  test_results[1] = test_writeEnable();
  test_results[2] = test_readRegisterOne();
  test_results[3] = test_quadEnable();
  test_results[4] = test_eraseSector();
  test_results[5] = test_writePage();
  test_results[6] = test_readSector();
  test_results[7] = test_writeSector();
  test_results[8] = test_readCustom();
  test_results[9] = test_writeCustom();

  printIndented("FLASH Core Tests");
  for (int i = 0; i < 10; i++) {
      printMsg("%s: %s\n", test_names[i], test_results[i] ? "GOOD" : "FAIL");
  }
}

