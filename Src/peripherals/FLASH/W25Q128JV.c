/*
 * W25Q128JV.c (FLASH interface)
 *
 * December 3, 2023
 *   Author: Anthony Surkov
 *
 * Last updated: 07-28-24
 */

#include "W25Q128JV.h"

extern int core_Hz; // from core_config.h

uint32_t flash_getPageAddr(uint32_t page) {
  if (page > FLASH_MAX_PAGE) { //MIN check not needed. Not possible with uint32_t
    return NULL;
  }
  return (uint32_t) (page * FLASH_PAGE_SIZE);
}

bool flash_writeSector(uint16_t sector, uint8_t* buffer) {
	if (sector > FLASH_MAX_SECTOR) {
		return false;
	}
	uint32_t page = sector * FLASH_PAGES_PER_SECTOR; //convert to pages

	for (uint8_t i = 0; i < 16; i++) {
		if (!flash_writePage(page+i, buffer)) {
			return false;
		}
		buffer += 256;
	}

	return flash_wait();
}

bool flash_readSector(uint16_t sector, uint8_t* buffer) {
	if (sector > FLASH_MAX_SECTOR) {
		return false;
	}
	uint32_t page = sector * FLASH_PAGES_PER_SECTOR; //convert sector to pages
	uint32_t address = flash_getPageAddr(page);

	flashSpi_command(QSPI_READ_DATA, true, address, NULL, buffer, 4096);

	return flash_wait();
}

bool flash_eraseSector(uint16_t sector) {
	if (sector > FLASH_MAX_SECTOR) {
		return false;
	}
	uint32_t page = sector * FLASH_PAGES_PER_SECTOR;
	uint32_t address = flash_getPageAddr(page);

	if (!flash_writeEnable()) {
		return false;
	}

	flashSpi_command(QSPI_SECTOR_ERASE, true, address, NULL, NULL, 0);

	return flash_wait();
}

bool flash_writePage(uint16_t page, uint8_t* buffer) {
	if (page > FLASH_MAX_PAGE) {
		return false;
	}
	uint32_t address = flash_getPageAddr(page);

	if (!flash_writeEnable()) {
		return false;
	}

	flashSpi_command(QSPI_PAGE, true, address, buffer, NULL, 256);

	return flash_wait();
}

bool flash_readPage(uint16_t page, uint8_t* buffer) {
	if (page > FLASH_MAX_PAGE) {
		return false;
	}
	uint32_t address = flash_getPageAddr(page);

	flashSpi_command(QSPI_READ_DATA, true, address, NULL, buffer, 256);

	return flash_wait();
}

bool flash_readCustom(uint32_t page, uint8_t* buffer, uint16_t size) {
	if (page > FLASH_MAX_PAGE) {
		return false;
	}
	uint32_t address = flash_getPageAddr(page);

	flashSpi_command(QSPI_READ_DATA, true, address, NULL, buffer, size);

  	return flash_wait();
}

bool flash_writeCustom(uint32_t page, uint8_t* userBuffer, uint16_t size) {
	uint16_t pagesToWrite = size / 256;
	uint16_t remainingBytes = size % 256;

	for (uint16_t i = 0; i < pagesToWrite; i++) {
		if (!flash_writePage(page + i, userBuffer)) {
			return false;
		}
		userBuffer += 256;
	}

	if ( remainingBytes > 0 ) {
		uint8_t userBuffer_tail[256];
		for (uint16_t j = 0; j < 256; j++) {
			if ( j < remainingBytes ) {
				userBuffer_tail[j] = userBuffer[j];
			}
			else {
				userBuffer_tail[j] = 0xFF;
			}
		}
		if (!flash_writePage(page + pagesToWrite, userBuffer_tail)) {
			return false;
		}
	}
	return flash_wait();
}

// Reads register one in full. Shared by flash_getStatus() and flash_writeEnable().
static void flash_readStatusOne(uint8_t* register_one) {
	flashSpi_command(QSPI_READ_REGISTER_ONE, false, 0, NULL, register_one, 1);
}

bool flash_writeEnable() {
	flashSpi_command(QSPI_WRITE_ENABLE, false, 0, NULL, NULL, 0);

	// The latch only sets if the chip decoded the command, so a missing or
	// miswired chip fails here instead of in the erase or program after it.
	uint8_t register_one = 0;
	flash_readStatusOne(&register_one);
	return register_one & FLASH_SR1_WEL;
}

bool flash_writeDisable() {
	flashSpi_command(QSPI_WRITE_DISABLE, false, 0, NULL, NULL, 0);

	return flash_wait();
}

bool flash_quadEnable() {
	uint8_t register_two;
	if (!flash_readRegisterTwo(&register_two)) {
		return false;
	}

	if ( (register_two & QSPI_QUAD_REGISTER) == QSPI_QUAD_REGISTER) {
		return true;
	}
	register_two |= (1 << 1);

	// Status register writes are ignored unless write enable is set first.
	if (!flash_writeEnable()) {
		return false;
	}

	flashSpi_command(QSPI_WRITE_REGISTER_TWO, false, 0, &register_two, NULL, 1);

	return flash_wait();
}

bool flash_readRegisterTwo(uint8_t* ptr_register_two) {
	flashSpi_command(QSPI_READ_REGISTER_TWO, false, 0, NULL, ptr_register_two, 1);

	return flash_wait();
}

bool flash_wait() {
	// The cycle counter is off after reset, and enabling it again is harmless.
	// getSysTime() can't be used: its timer starts after fs_mount() at boot.
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

	uint32_t start = DWT->CYCCNT;
	uint32_t timeout = FLASH_WAIT_TIMEOUT_MS * (core_Hz / 1000);
	while (flash_getStatus() != 0) {
		if (DWT->CYCCNT - start > timeout) {
			return false;
		}
	}
	return true;
}

uint8_t flash_getStatus() {
	// A missing chip reads 0xFF through the MISO pull-up, so it reads as busy
	// and callers never mistake it for a ready chip.
	uint8_t register_one;
	flash_readStatusOne(&register_one);
	return register_one & FLASH_SR1_BUSY;
};
