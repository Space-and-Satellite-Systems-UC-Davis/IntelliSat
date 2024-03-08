/*
 * W25Q128JV.c (FLASH interface)
 *
 * - December 3, 2023
 * - Author: Anthony Surkov
 *
 * - Last updated: 01-22-24
 * SUBJECT TO CHANGE; WIP
 */

#include "W25Q128JV.h"

//rev1, works
void* flash_findPage(uint32_t page) {
  uint8_t block = page / PAGES_PER_BLOCK;
  page = page % PAGES_PER_BLOCK;
  uint8_t sector = page / PAGES_PER_SECTOR;
  page = page % PAGES_PER_SECTOR;

  void* memPtr = (uint32_t*) (
    (block * BLOCK_SIZE) +
    (sector * SECTOR_SIZE) +
    (page * PAGE_SIZE)
  );

  return memPtr;
}

//rev1, untested
bool flash_writeSector(uint16_t sector, uint8_t* buffer) {
  if (qspi_getStatus() == QSPI_BUSY) {
    return 0;
  }
  if (sector > MAX_SECTOR) {
    return 0;
  }
  sector *= SECTOR_SIZE; //convert to pages

  for (int i = 0; i < 16; i++, sector++) {
    flash_writeEnable();
    flash_writePage(sector, buffer);
    sector++;
    buffer += 256;
  }
  return 1;
}

bool flash_readSector(uint16_t sector, uint8_t* buffer) {
  if (qspi_getStatus() == QSPI_BUSY) {
    return 0;
  }
  if (sector > MAX_SECTOR) {
    return 0;
  }
  sector *= SECTOR_SIZE;

  for (int i = 0; i < 16; i++, sector++) {
    flash_writeEnable();
    flash_writePage(sector, buffer);
    sector++;
    buffer += 256;
  }
  return 1;
}

bool flash_eraseSector(uint16_t sector) {
  if (qspi_getStatus() == QSPI_BUSY) {
    return 0;
  }
  if (sector > MAX_SECTOR) {
    return 0;
  }
  sector *= PAGES_PER_SECTOR; //convert to page
  uint32_t address = (uint32_t)flash_findPage(sector);

  flash_writeEnable();

  qspi_setCommand(
      QSPI_FMODE_INDIRECT_WRITE,
      QSPI_1_WIRE,
      QSPI_1_WIRE,
      QSPI_UNUSED,
      0,
      QSPI_UNUSED,
      false
  );
  qspi_sendCommand(
      QSPI_SECTOR_ERASE,
      address,
      0,
      0,
      QSPI_1_WIRE,
      QSPI_TIMEOUT_PERIOD
  );

  while (qspi_getStatus() == QSPI_BUSY);

  return 1;
}

bool flash_writePage(uint16_t page, uint8_t* buffer) {
  if (qspi_getStatus() == QSPI_BUSY) {
    return 0;
  }

  uint32_t address = (uint32_t)flash_findPage(page);

  flash_writeEnable();

  qspi_setCommand(
      QSPI_FMODE_INDIRECT_WRITE,
      QSPI_1_WIRE,
      QSPI_1_WIRE,
      QSPI_UNUSED,
      0,
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

  while (qspi_getStatus() == QSPI_BUSY);

  return 1;
}

bool flash_readPage(uint16_t page, uint8_t* buffer) {
  if (qspi_getStatus() == QSPI_BUSY) {
    return 0;
  }

  uint32_t address = (uint32_t)flash_findPage(page);

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
    0x03,
    address,
    256,
    buffer,
    0,
    QSPI_TIMEOUT_PERIOD
  );

  return 1;
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

bool flash_writeCustom(uint16_t size, uint32_t page, uint8_t* buffer) {
  if (qspi_getStatus() == QSPI_BUSY) {
    return 0;
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

bool flash_writeDisable() {
	if (qspi_getStatus() == QSPI_BUSY) {
		return 1;
	}
	qspi_setCommand(
		QSPI_FMODE_INDIRECT_WRITE,
		QSPI_1_WIRE,
		QSPI_UNUSED,
		QSPI_UNUSED,
		0,
		QSPI_UNUSED,
		false
	);
	qspi_sendCommand(
		0x04, //write disable
		QSPI_UNUSED,
		0,
		0,
		QSPI_WRITE,
		QSPI_TIMEOUT_PERIOD
	);
	while (qspi_getStatus() == QSPI_BUSY);
	return 0;
}

bool flash_writeEnable() {
  if (qspi_getStatus() == QSPI_BUSY) {
    return 1;
  }
  qspi_setCommand(
      QSPI_FMODE_INDIRECT_WRITE,
      QSPI_1_WIRE,
      QSPI_UNUSED,
      QSPI_UNUSED,
      0,
      QSPI_UNUSED,
      false
  );
  qspi_sendCommand(
      QSPI_WRITE_ENABLE,
      QSPI_UNUSED,
      0,
      0,
      QSPI_WRITE,
      QSPI_TIMEOUT_PERIOD
  );

  qspi_setCommand(
      QSPI_FMODE_AUTOMATIC_POLLING,
      QSPI_1_WIRE,
      QSPI_UNUSED,
      QSPI_UNUSED,
      0,
      QSPI_UNUSED,
      false
  );
  /*
  qspi_statusPoll(
      0,
      0x05,
      QSPI_WRITE_REGISTER,
      QSPI_WRITE_REGISTER,
      QSPI_TIMEOUT_PERIOD
  );
  */
  while (qspi_getStatus() == QSPI_BUSY);
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
/* Interestingly, this status poll seems not to get stuck.
 * Compare it with writeEnable's (it got stuck in testing)
  qspi_setCommand(
      QSPI_FMODE_INDIRECT_READ,
      QSPI_1_WIRE,
      QSPI_UNUSED,
      QSPI_UNUSED,
      0,
      QSPI_UNUSED,
      false
  );
  qspi_statusPoll(
      0,
      QSPI_READ_REGISTER_TWO,
      QSPI_QUAD_REGISTER,
      QSPI_QUAD_REGISTER,
      QSPI_TIMEOUT_PERIOD
  );
*/
  while (qspi_getStatus() == QSPI_BUSY);

  return 0;
}

void flash_readRegisterTwo(uint8_t* ptr_register_two) {
  if (qspi_getStatus() == QSPI_BUSY) {
    return;
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
      QSPI_READ_REGISTER_TWO,
      0,
      1,
      ptr_register_two,
      0,
      QSPI_TIMEOUT_PERIOD
  );
}

bool flash_wait() {
  while (flash_getStatus() != 0) {}
  return 0;
};

//wtf?
uint8_t flash_getStatus() {
  if (qspi_getStatus() == QSPI_BUSY) {
    return 1;
  }

  qspi_setCommand(
    QSPI_FMODE_INDIRECT_READ,
    QSPI_1_WIRE,
    QSPI_UNUSED,
    QSPI_UNUSED,
    0,
    QSPI_UNUSED,
    false
  );

  uint8_t register_one;
  qspi_sendCommand(
    0x05,
    0x00,
    8,
    &register_one,
    0,
    QSPI_TIMEOUT_PERIOD
  );
  return register_one & (1 << 0);
};

