#include "print_scan.h"
#include "FLASH/W25Q128JV.h"

/*
 * For now this won't be implemented on the main branch
 * Therefore, the main branch cannot be built.
 * Each development / feature branch has it's own implementation
 * which is used for testing specific features.
 */

/*
 * FUNCTIONS TESTED SO FAR
 * DONE readJedecID()
 * - expected: 00 00 00 -> EF 17 9F
 * - actual: 00 00 00 -> EF 17 9F
 *
 * DONE readRegisterOne(), writeEnable()
 * - is able to read registerOne. verified concurrently with writeEnable().
 * - expected: 00000000 -> 00000010
 * - actual: 00000000 -> 00000010
 *
 * DONE writeDisable()
 * (tested after writeEnable)
 * - expected: 00000010 -> 00000000
 * - actual: 00000010 -> 00000000
 *
 * DONE readPage()
 * (tested with writePage)
 * - expected: all 0xFF in random location. all alternating 0's and 1's where written to
 * - actual: matches
 *
 * DONE writePage()
 * - expected: all alternating 0's and 1's in location written to
 * - actual: matches
 *
 * DONE sectorRead()
 * - expected: pageRead matches sectorRead output
 * - actual: true
 */


//accessory fxn for testing
void printBinary(uint8_t byte) {
  for (int i = 7; i >= 0; i--) {
    printMsg("%c", (byte & (1 << i)) ? '1' : '0');
  }
  printMsg("\n\r");
}

//DONE
void test_writePage() {
	uint16_t page = 900;

	//let's check the contents of the page before writing to it
	uint8_t verification_buffer[256];
	for (int i = 0; i < 256; i++) {
		verification_buffer[i] = 0;
	}
	printMsg("\n\n\r");
	printMsg("This is the empty buffer we're going to use to read the page first.\n\r");
	printMsg("We will be reading page ");
	printMsg("%u", page);
	printMsg("\n\r");
	for (int i = 0; i < 256; i++) {
		printMsg("%u", (unsigned int)verification_buffer[i]);
		printMsg(" ");
	}
	flash_readPage(page, verification_buffer);
	printMsg("\n\n\r");
	printMsg("This is the contents of that page.");
	printMsg("\n\r");
	for (int i = 0; i < 256; i++) {
		printMsg("%u", (unsigned int)verification_buffer[i]);
		printMsg(" ");
	}

	//set new buffer to random nonzero shit (alternating 4's and 5's, starting with 5
	uint8_t writing_buffer[256] = { 0 };
	for (int i = 0; i < 256; i++) {
		if ( (i % 2) == 0) {
			writing_buffer[i] = 0;
		}
		else {
			writing_buffer[i] = 1;
		}
	}
	printMsg("\n\n\r");
	printMsg("This is what we're going to be writing to it now:");
	printMsg("\n\r");
	for (int i = 0; i < 256; i++) {
		printMsg("%u", (unsigned int)writing_buffer[i]);
		printMsg(" ");
	}

	//write it
	flash_writePage(page, writing_buffer);

	//DEV NOTES
	//MAKE SYSTEM WAIT UNTIL WRITE IS COMPLETE BEFORE READING. CURRENTLY READS CONCURRENTLY TO WRITING

	uint8_t verification_2[256] = { 0 };
	printMsg("\n\n\r");
	printMsg("This is the empty buffer we're going to use to read the page after writing.\n\r");
	for (int i = 0; i < 256; i++) {
		printMsg("%u", (unsigned int)verification_2[i]);
		printMsg(" ");
	}
	flash_readPage(page, verification_2);
	printMsg("\n\n\r");
	printMsg("This is the contents of the page we wrote to.");
	printMsg("\n\r");
	for (int i = 0; i < 256; i++) {
		printMsg("%u", (unsigned int)verification_2[i]);
		printMsg(" ");
	}

}

//accessory for testing quadEnable. not sure if should integrate into primary Whatever.c
//wtf? not working
bool flash_quadDisable() {
	if (qspi_getStatus() == QSPI_BUSY) {
	    return false;
	  }

	  uint8_t register_two;
	  flash_readRegisterTwo(&register_two);

	  if ( (register_two & QSPI_QUAD_REGISTER) == 0b00000000) {
	    return 0;
	  }

	  register_two &= ~(1 << 1);

	  qspi_setCommand(
	      QSPI_FMODE_INDIRECT_WRITE,
	      QSPI_1_WIRE,
	      QSPI_UNUSED,
	      QSPI_UNUSED,
	      0,
	      QSPI_1_WIRE,
	      false
	  );
	  qspi_sendCommand(
	      QSPI_WRITE_REGISTER_TWO,
	      0,
	      1,
	      &register_two,
	      1,
	      QSPI_TIMEOUT_PERIOD
	  );

	  while (qspi_getStatus() == QSPI_BUSY);

	  return 0;
}

//currently not working.
//quadDisable seems to be faulty, but am not sure. since I cannot see if quadEnable is working
void test_quadToggle() {

  uint8_t register_two;

  printMsg("\n\n\r");
  printMsg("Qinit:");
  flash_readRegisterTwo(&register_two);
  printBinary(register_two);

  flash_quadDisable();
  printMsg("\n\r");
  printMsg("QDisable:");
  flash_readRegisterTwo(&register_two);
  printBinary(register_two);

  flash_quadEnable();
  printMsg("\n\r");
  printMsg("QEnable:");
  flash_readRegisterTwo(&register_two);
  printBinary(register_two);

  flash_quadDisable();
  printMsg("\n\r");
  printMsg("QDisable:");
  flash_readRegisterTwo(&register_two);
  printBinary(register_two);
  printMsg("\n\r");

}

//DONE
void flash_readRegisterOne(uint8_t* register_one) {
  if (qspi_getStatus() == QSPI_BUSY) {
    printMsg("busy!\n\r");
  }
  qspi_setCommand(
      QSPI_FMODE_INDIRECT_READ,
      QSPI_1_WIRE,
      QSPI_UNUSED,
      QSPI_UNUSED,
      0,
      1,
      false
  );
  qspi_sendCommand(
      0x05, //read register 1
      QSPI_UNUSED,
      1,
      register_one,
      QSPI_READ,
      QSPI_TIMEOUT_PERIOD
  );
}

//DONE
void read_jedecID() {
  uint8_t id_receiver[3] = { 0 };

  //formatting + init case
  printMsg("init id_receiver var: \n\r");
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
      0, //0 dummy cycles for JEDEC
      1, //1 wire for data
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

  //formatting + test case
  printMsg("final id_receiver var: \n\r");
  for (int i = 0; i < 3; i++) {
 	  printMsg("%02x", (unsigned int)id_receiver[i]);
 	  printMsg(" ");
  }
  printMsg("\n\r");
}

//DONE
void test_writeEnable() {
  uint8_t register_one = 0;
  //flash_writeDisable();

  flash_readRegisterOne(&register_one);
  printMsg("\n\r");
  printMsg("init register one:");
  printBinary(register_one);
  printMsg("\n\r");

  flash_writeEnable();

  flash_readRegisterOne(&register_one);
  printMsg("\n\r");
  printMsg("final register one:");
  printBinary(register_one);
  printMsg("n\r");
}

//DONE
void test_readRegisterOne() {
	uint8_t register_one = 0;
	printMsg("\n\r");
	printMsg("init register one:");
	printBinary(register_one);

	flash_readRegisterOne(&register_one);
	printMsg("\n\r");
	printMsg("final register one:");
	printBinary(register_one);
}

void test_readRegisterTwo() {
	uint8_t register_two = 0;
	printMsg("\n\r");
	printMsg("init register two:");
	printMsg("%u\n\r", register_two);

	flash_readRegisterOne(&register_two);
	printMsg("\n\r");
	printMsg("final register one:");
	printMsg("\n\r");
	printMsg("%u\n\r", register_two);
}

bool test_readSector(uint32_t sector) {
	//read sector with readSector
	uint8_t sectorBuffer[4096] = { 0 };
	flash_readSector(sector, sectorBuffer);

	//read sector with readPage and compare it to sectorBuffer
	uint8_t pageBuffer[256] = { 0 };
	uint32_t page = sector * 16;

	//for comparison purposes
	uint8_t* ptr_sectorBuffer = sectorBuffer;

	for (int i = 0; i < 16; i++, page++) {
	  flash_readPage(page, pageBuffer);
	  for (int i = 0; i < 256; i++, ptr_sectorBuffer++) {
	    if (*ptr_sectorBuffer != pageBuffer[i]) {
	      return false;
	    }
	  }
	}
	return true;
}

bool test_writeSector(uint32_t sector) {
  //fill a 4096-byte buffer with alternating 4's and 5's, and a 256 buffer with 1's and 2's
  uint8_t pageOut[256];
  uint8_t sectorOut[4096];
  for (int i = 0; i < 256; i++) {
	  if (i % 2 == 0) {
		  pageOut[i] = 2;
	  }
	  else {
		  pageOut[i] = 1;
	  }
  }
  for (int i = 0; i < 4096; i++) {
	  if (i % 2 == 0) {
		  sectorOut[i] = 4;
	  }
	  else {
		  sectorOut[i] = 5;
	  }
  }

  uint8_t sectorIn[4096];

  // flash_writeSector(sector, sectorOut);
  //flash_readSector(sector, sectorIn);
/*
  printMsg("\n\r");
  for (int i = 0; i < 4096; i++) {
	  if (i % 256 == 0) {
		  printMsg("\n\r");
		  printMsg("Page #");
		  printMsg("%u", i / 256);
		  printMsg("\n\r");
	  }
	  printMsg("%u", sectorIn[i]);
	  printMsg(" ");
  }
*/
  uint8_t pageIn[256];
  uint32_t page = sector * 16;

  for (int i = 0; i < 16; i++) {
	  flash_writePage(page + i, pageOut);
  }
  for (int i = 0; i < 16; i++) {
	  flash_readPage(page + i, pageIn);
	  printMsg("\n\r");
	  printMsg("Page #");
	  printMsg("%u", i);
	  printMsg("\n\r");
	  for (int j = 0; j < 256; j++) {
		  printMsg("%u", pageIn[i]);
		  printMsg(" ");
	  }
  }

  return true;
}

void branch_main() {
	qspi_config(23, 2, 0); //temporary solution

//	test_writeSector(1000);

	read_jedecID();
}
