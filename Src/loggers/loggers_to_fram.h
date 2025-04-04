/*
 * logger_to_flash.h (Logger-to-FLASH Interface)
 *
 * Authors: Yalamber Subba
 * Last updated: 2025.4.4
 * Description: FRAM utilities belonging to the loggers system
 */

#define FRAM_PAGE_SIZE 256
#define FRAM_SECTOR_SIZE 256


uint8_t FRAM_pushData(uint16_t page, void *data, size_t data_size);
uint8_t FRAM_pushData(uint16_t page, void *data, size_t data_size);
