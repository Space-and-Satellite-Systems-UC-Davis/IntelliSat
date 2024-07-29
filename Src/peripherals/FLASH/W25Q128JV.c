/*
 * W25Q128JV.c (FLASH interface)
 *
 * - December 3, 2023
 * - Author: Anthony Surkov
 *
 * - Last updated: 07-28-24
 * SUBJECT TO CHANGE; WIP
 */

#include "W25Q128JV.h"

	void* flash_findPage(uint32_t page) {
	uint8_t block = page / FLASH_PAGES_PER_BLOCK;
	page = page % FLASH_PAGES_PER_BLOCK;
	uint8_t sector = page / FLASH_PAGES_PER_SECTOR;
	page = page % FLASH_PAGES_PER_SECTOR;

	void* memPtr = (uint32_t*) (
			(block * FLASH_BLOCK_SIZE) +
			(sector * FLASH_SECTOR_SIZE) +
			(page * FLASH_PAGE_SIZE)
	);

	return memPtr;
}

bool flash_writeSector(uint16_t sector, uint8_t* buffer) {
	if (qspi_getStatus() == QSPI_BUSY) {
		return false;
	}
	if (sector > FLASH_MAX_SECTOR) {
		return false;
	}
	sector *= FLASH_SECTOR_SIZE; //convert to pages

	for (int i = 0; i < 16; i++, sector++) {
		flash_writeEnable();
		flash_writePage(sector, buffer);
		sector++;
		buffer += 256;
	}

	flash_wait();
	return true;
}

bool flash_readSector(uint16_t sector, uint8_t* buffer) {
	if (qspi_getStatus() == QSPI_BUSY) {
		return false;
	}

	uint32_t page = sector * FLASH_SECTOR_SIZE; //convert sector to pages
	uint32_t address = (uint32_t)flash_findPage(page);

	qspi_setCommand(
		QSPI_FMODE_INDIRECT_READ, //Instruction type
		QSPI_1_WIRE, //Number of wires for Instruction Phase
		QSPI_1_WIRE, //Number of wires for Address Phase
		QSPI_UNUSED, //Number of wires for Alternative Bytes Phase
		QSPI_UNUSED, //Number of dummy cycles
	    QSPI_1_WIRE, //Number of wires for Data Phase
	    false //DMA in use?
	);
	qspi_sendCommand(
	    QSPI_READ_DATA, //Instruction
	    address, //Address
	    4096, //Data size (bytes)
	    buffer, //Buffer where data is located/will be stored
	    QSPI_READ, //Read or write?
	    QSPI_TIMEOUT_PERIOD //Timeout period
	);

	flash_wait();
	return true;
}

bool flash_eraseSector(uint16_t sector) {
	if (qspi_getStatus() == QSPI_BUSY) {
		return false;
	}
	if (sector > FLASH_MAX_SECTOR) {
		return false;
	}
	sector *= FLASH_PAGES_PER_SECTOR;
	uint32_t address = (uint32_t)flash_findPage(sector);

	flash_writeEnable();

	qspi_setCommand(
		QSPI_FMODE_INDIRECT_WRITE,
		QSPI_1_WIRE,
		QSPI_1_WIRE,
		QSPI_UNUSED,
		QSPI_UNUSED,
		QSPI_UNUSED,
		false
	);
	qspi_sendCommand(
		QSPI_SECTOR_ERASE,
		address,
		QSPI_UNUSED,
		QSPI_UNUSED,
		QSPI_WRITE,
		QSPI_TIMEOUT_PERIOD
	);

	flash_wait();
	return true;
}

bool flash_writePage(uint16_t page, uint8_t* buffer) {
	if (qspi_getStatus() == QSPI_BUSY) {
		return false;
	}

	uint32_t address = (uint32_t)flash_findPage(page);

	flash_writeEnable();

	qspi_setCommand(
		QSPI_FMODE_INDIRECT_WRITE,
		QSPI_1_WIRE,
		QSPI_1_WIRE,
		QSPI_UNUSED,
		QSPI_UNUSED,
		QSPI_1_WIRE,
		false
	);
	qspi_sendCommand(
		QSPI_PAGE,
		address,
		256,
		buffer,
		QSPI_WRITE,
		QSPI_TIMEOUT_PERIOD
	);

	flash_wait();
	return true;
}

bool flash_readPage(uint16_t page, uint8_t* buffer) {
	if (qspi_getStatus() == QSPI_BUSY) {
		return false;
	}

	uint32_t address = (uint32_t)flash_findPage(page);

	qspi_setCommand(
		QSPI_FMODE_INDIRECT_READ,
		QSPI_1_WIRE,
		QSPI_1_WIRE,
		QSPI_UNUSED,
		QSPI_UNUSED,
		QSPI_1_WIRE,
		false
	);
	qspi_sendCommand(
		0x03,
		address,
		256,
		buffer,
		QSPI_READ,
		QSPI_TIMEOUT_PERIOD
	);

	flash_wait();
	return true;
}
/*bool flash_readCustom(uint16_t size, uint16_t page, uint8_t* buffer) {
	if (qspi_getStatus() == QSPI_BUSY) {
    	return 1;
  	}
  	qspi_setCommand(
      	QSPI_FMODE_INDIRECT_READ,
      	QSPI_1_WIRE,
      	QSPI_1_WIRE,
      	QSPI_UNUSED,
      	0,
      	QSPI_1_WIRE,
      	false
  	);
  	qspi_sendCommand(
      	QSPI_READ_DATA,
      	(uint32_t)flash_findPage(page),
      	size,
      	buffer,
      	0,
      	QSPI_TIMEOUT_PERIOD
  	);
  //INCOMPLETE (?)
  	return 1;
}*/

//terrible
bool flash_writeCustom(uint16_t size, uint32_t page, uint8_t* buffer) {
	if (qspi_getStatus() == QSPI_BUSY) {
		return false;
	}
	if (size > 256) {
		uint8_t full_pages = size / 256;
		for (; full_pages > 0; full_pages--) {
			flash_writeEnable();
			flash_writePage(page, buffer);
			page++;
			buffer += 256;
		}
		size = size % 256;
		if (size == 0) { return 1; }
	}
	uint8_t temp_page[256] = { 0 };
	for (int i = 0; size > 0; size--, buffer++, i++) {
		temp_page[i] = *buffer;
	}
	flash_writeEnable();
	flash_writePage(page, temp_page);

	return 1;
}

bool flash_writeEnable() {
	if (qspi_getStatus() == QSPI_BUSY) {
		return false;
	}
	qspi_setCommand(
		QSPI_FMODE_INDIRECT_WRITE,
		QSPI_1_WIRE,
		QSPI_UNUSED,
		QSPI_UNUSED,
		QSPI_UNUSED,
		QSPI_UNUSED,
		false
	);
	qspi_sendCommand(
		QSPI_WRITE_ENABLE,
		QSPI_UNUSED,
		QSPI_UNUSED,
		QSPI_UNUSED,
      	QSPI_WRITE,
		QSPI_TIMEOUT_PERIOD
	);

	flash_wait();
	return 0;
}

bool flash_quadEnable() {
	if (qspi_getStatus() == QSPI_BUSY) {
		return false;
	}

	uint8_t register_two;
	flash_readRegisterTwo(&register_two);

	if ( (register_two & QSPI_QUAD_REGISTER) == QSPI_QUAD_REGISTER) {
		return 0;
	}
	register_two |= (1 << 1);

	qspi_setCommand(
		QSPI_FMODE_INDIRECT_WRITE,
		QSPI_1_WIRE,
		QSPI_UNUSED,
		QSPI_UNUSED,
		QSPI_UNUSED,
		QSPI_1_WIRE,
		false
	);
	qspi_sendCommand(
		QSPI_WRITE_REGISTER_TWO,
		QSPI_UNUSED,
		1,
		&register_two,
		QSPI_WRITE,
		QSPI_TIMEOUT_PERIOD
	);

	flash_wait();
 	return 0;
}

void flash_readRegisterTwo(uint8_t* ptr_register_two) {
	//Subsidiary function. QSPI busy check not needed
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
		QSPI_READ_REGISTER_TWO,
		QSPI_UNUSED,
		1,
		ptr_register_two,
		QSPI_READ,
		QSPI_TIMEOUT_PERIOD
	);

	flash_wait();
}

bool flash_wait() {
	while (flash_getStatus() != 0) {}
	return true;
}

uint8_t flash_getStatus() {
	if (qspi_getStatus() == QSPI_BUSY) {
		return false;
	}

	qspi_setCommand(
		QSPI_FMODE_INDIRECT_READ,
		QSPI_1_WIRE,
		QSPI_UNUSED,
		QSPI_UNUSED,
		QSPI_UNUSED,
		QSPI_1_WIRE,
		false
	);

	uint8_t register_one;
	qspi_sendCommand(
		QSPI_READ_REGISTER_ONE,
		0,
		1,
		&register_one,
		QSPI_READ,
		QSPI_TIMEOUT_PERIOD
	);
	return register_one & (1 << 0);
};
