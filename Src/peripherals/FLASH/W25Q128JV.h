/*
 * W25Q128JV.h (FLASH interface)
 *
 * - December 3, 2023
 *    Author: Anthony Surkov
 *
 * Most recently updated 07-18-24 for reorganization
 */

#ifndef REALOP1_PERIPHERALS_FLASH_H_
#define REALOP1_PERIPHERALS_FLASH_H_

#include "../../system_config/QSPI/QSPI.h"

//Register and mask macros
#define QSPI_READ_REGISTER_TWO    0x35
#define QSPI_READ_REGISTER_ONE    0x05
#define QSPI_WRITE_REGISTER_TWO   0x31
#define QSPI_QUAD_REGISTER        0b00000010
#define QSPI_WRITE_ENABLE         0x06
#define QSPI_WRITE_REGISTER       0b00000010
#define QSPI_READ_DATA            0x03
#define QSPI_PAGE                 0x02
#define PAGE_MASK                 0x11111100
#define QSPI_READ_DATA            0x03
#define QSPI_SECTOR_ERASE         0x20

//Memory organization macros
#define FLASH_BLOCK_SIZE          65536
#define FLASH_SECTOR_SIZE         4096
#define FLASH_PAGE_SIZE           256

#define FLASH_PAGES_PER_BLOCK     256
#define FLASH_PAGES_PER_SECTOR    16

#define FLASH_MIN_PAGE            0
#define FLASH_MAX_PAGE            65535
#define FLASH_MIN_SECTOR          0
#define FLASH_MAX_SECTOR          4095


/**
 * Converts a page number (0 ~ 65535) to its memory address in the FLASH.
 * Specifying a page greater than 65535 will override page 65535.
 * Specifying a page less than 0 will override page 0.
 *
 * @returns Void pointer to the address.
 */
void* flash_findPage(uint32_t page);


/**
 * Reads one page.
 *
 * @param page     # of page (0 ~ 65536) to read from. Do not use address.
 * @param buffer   Buffer to put data into. Ensure appropriate size (256 bytes).
 *
 * @returns Whether read was successfully completed (boolean).
 */
bool flash_readPage(uint16_t page, uint8_t* buffer);


/**
 * Writes one page to a previously erased location (use flash_sectorErase).
 *
 * @param page     # of page (0 ~ 65535) to write to. Do not use address.
 * @param buffer   Buffer to take data from. Ensure appropriate size (256 bytes).
 *
 * @returns Whether write was successfully completed.
 */
bool flash_writePage(uint16_t page, uint8_t* buffer);


/**
 * Reads one sector.
 *
 * @param sector   # of sector (0 ~ 4095) to read from. Do not use address.
 * @param buffer   Buffer to put data into. Ensure appropriate size (4096 bytes).
 *
 * @returns Whether read was successfully completed.
 */
bool flash_readSector(uint16_t sector, uint8_t* buffer);


/**
 * Writes one sector to a previously erased location (use flash_sectorErase).
 *
 * @param sector   # of sector (0 ~ 4095) to write to. Do not use address.
 * @param buffer   Buffer to take data from. Ensure appropriate size (4096 bytes).
 *
 * @returns Whether write was successfully completed.
 */
bool flash_writeSector(uint16_t sector, uint8_t* buffer);


/**
 * Erases one sector.
 *
 * @param sector   # of sector (0 ~ 4095) to erase. Do not use address.
 *
 * @returns Whether erase was successfully completed.
 */
bool flash_eraseSector(uint16_t sector);


/**
 * Reads a variable number of bytes.
 *
 * @param size     # of bytes to read.
 * @param page     Page to start reading from. Do not use address.
 * @param buffer   Buffer to put data into. Ensure appropriate size (variable).
 *
 * @returns Whether read was successfully completed.
 */
bool flash_readCustom(uint16_t size, uint32_t page, uint8_t* buffer);


/**
 * Writes a variable number of bytes.
 * WARNING - uses large amount of memory to execute. Max 256 bytes of RAM.
 *
 * @param size     # of bytes to write.
 * @param page     Page to start writing from. Do not use address.
 * @param buffer   Buffer to take data from. Ensure appropriate size (variable).
 */
bool flash_writeCustom(uint16_t size, uint32_t page, uint8_t* buffer);


/**
 * Writes '1' to write_enable bit on register one. i.e. enables write functionality.
 * Is already built into functions that need it. Generally, do not use.
 *
 * @returns whether write was successfully enabled.
 */
bool flash_writeEnable();


/**
 * Writes '1' to quad_enable bit on register two. i.e. enables QSPI functionality.
 * Is mostly unnecessary for currently written functions (07-28-24).
 *
 * @returns whether QSPI was successfully enabled.
 */
bool flash_quadEnable();


/**
 * Reads the contents of register two. Auxiliary function to flash_quadEnable().
 *
 * @param ptr_register_two    Reads the contents of register two into argued pointer.
 */
void flash_readRegisterTwo(uint8_t* ptr_register_two);


/**
 * Waits for get_flash_status() to return 0. i.e, waits until flash is available.
 *
 * @returns true when flash available.
 */
bool flash_wait();


/**
 * Reads register one's busy flag. i.e. finds whether flash is currently busy.
 *
 * @returns masked register one (00000010 if busy, 00000000 if not).
 */
uint8_t flash_getStatus();



#endif /* PERIPHERALS_FLASH_W25Q128JV_H_ */
