#include "print_scan.h"
#include "FLASH/W25Q128JV.h"

/*
 * branch_main .c (FLASH testing)
 *
 * - July 29, 2024
 * - Author: Anthony Surkov
 *
 * - Last updated: 07-29-24
 */

//General helper functions
void printBinary(uint8_t byte) {
	for (int i = 7; i >= 0; i--) {
		printMsg("%c", (byte & (1 << i)) ? '1' : '0');
	}
	printMsg("\n\r");
}

void printBuf(uint8_t buf[], int size) {
	printMsg("\n\r");
	for (int i = 0; i < size; i++) {
		printMsg("%u", (unsigned int)buf[i]);
		printMsg(" ");
	}
	printMsg("\n\r");
}

void printIndented(char* printme) {
	printMsg("\n\r");
	printMsg("%u", printme);
	printMsg("\n\r");
}

void fillBuf(uint8_t buf[], int size, int x) {
	for (int i = 0; i < size; i++) {
		buf[i] = x;
	}
}

void clearBuf(uint8_t buf[], int size) {
	for (int i = 0; i < size; i++) {
		buf[i] = 0;
	}
}

// DONE readJedecID()
// Goal: Basic verification of communication: retrieving manufacturer ID.
// * expected: 00 00 00 -> EF 17 9F
// * actual: 00 00 00 -> EF 17 9F
void test_readJedecID() {
	uint8_t id_receiver[3];

	printMsg("Empty ID receiver: \n\r");
	for (int i = 0; i < 3; i++) {
		printMsg("%02x", (unsigned int)id_receiver[i]);
		printMsg(" ");
	}
	printMsg("\n\r");

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

	printMsg("Full ID receiver: \n\r");
	for (int i = 0; i < 3; i++) {
	 	printMsg("%02x", (unsigned int)id_receiver[i]);
	 	printMsg(" ");
	}
	printMsg("\n\r");
}

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

//
void test_quadToggle() { }

//Helper function to verifying flash_writeEnable()
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

//See below
void test_readRegisterOne() {
	uint8_t register_one = 0;
	printIndented("Empty register one:");
	printBinary(register_one);

	flash_readRegisterOne(&register_one);
	printIndented("Full register one:");
	printBinary(register_one);
}

// DONE readRegisterOne(), writeEnable()
// Goal: read registerOne. Verify writeEnable() functionality for page tests.
// * expected: 00000000 -> 00000010
// * actual: 00000000 -> 00000010
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

//
void test_readRegisterTwo() {
	uint8_t register_two = 0;

	printIndented("Empty register two:");
	printBinary(register_two);

	flash_readRegisterOne(&register_two);

	printIndented("Full register two:");
	printBinary(register_two);
}

//Helper function to test_readSector. Fills a sector incrementally.
void testing_fillSector(uint16_t sector) {
	uint32_t page = sector * FLASH_PAGES_PER_SECTOR;

	for (int i = 0; i < 16; i++) {
		uint8_t bufferWrite[FLASH_PAGE_SIZE];
		fillBuf(bufferWrite, FLASH_PAGE_SIZE, i);

		flash_writePage(page+i, bufferWrite);
	}
}

//
bool test_readSector(uint16_t sector) {
	flash_eraseSector(sector);
	testing_fillSector(sector);

	uint8_t sectorBuffer[FLASH_SECTOR_SIZE];
	flash_readSector(sector, sectorBuffer);

	uint8_t pageBuffer[FLASH_PAGE_SIZE];
	uint32_t page = sector * FLASH_PAGES_PER_SECTOR;

	for (int i = 0; i < FLASH_PAGES_PER_SECTOR; i++, page++) {
		flash_readPage(page, pageBuffer);
		for (int j = 0; j < FLASH_PAGE_SIZE; j++) {
			if ( pageBuffer[j] != sectorBuffer[i*FLASH_PAGE_SIZE+j] ) {
				return false;
			}
		}
	}
	printBuf(sectorBuffer, FLASH_SECTOR_SIZE);
	return true;
}

//
bool test_writeSector(uint32_t sector) {
  uint8_t bufferWrite_Sector[FLASH_SECTOR_SIZE];
  uint8_t bufferRead_Sector_A[FLASH_SECTOR_SIZE];
  uint8_t bufferRead_Sector_B[FLASH_SECTOR_SIZE];

  flash_eraseSector(sector);
  testing_fillSector(sector);
  flash_readSector(sector, bufferRead_Sector_A);

  fillBuf(bufferWrite_Sector, FLASH_SECTOR_SIZE, 6);
  flash_eraseSector(sector);
  flash_writeSector(sector, bufferWrite_Sector);
  flash_readSector(sector, bufferRead_Sector_B);

  for (int i = 0; i < FLASH_SECTOR_SIZE; i++) {
	  if ( bufferRead_Sector_A[i] != bufferRead_Sector_B[i] ) {
		  return false;
	  }
  }
  printBuf(bufferRead_Sector_A, FLASH_SECTOR_SIZE);

  return true;
}

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

void branch_main() {

}
