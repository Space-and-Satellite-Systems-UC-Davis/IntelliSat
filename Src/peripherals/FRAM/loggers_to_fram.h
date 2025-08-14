/*
 * logger_to_fram.h (Logger-to-FRAM Interface)
 *
 * Authors: Yalamber Subba, Anthony Surkov
 * Last updated: 2025.05.12
 * Description: FRAM utilities belonging to the loggers system
 */

#include "../peripherals/FRAM/MB85RS256B.h"

#define FRAM_PAGE_SIZE         256
#define FRAM_SECTOR_SIZE       4096

#define FRAM_SUNSENSORS_PAGE   1
#define FRAM_MAG_PAGE          2
#define FRAM_IMU_PAGE          3
#define FRAM_TLE_PAGE          4

#define FRAM_SUNSENSORS_SIZE   96
#define FRAM_MAG_SIZE          48
#define FRAM_IMU_SIZE          48
#define FRAM_TLE_SIZE          140


uint8_t FRAM_fetchData(uint16_t page, size_t data_size, void *data);
uint8_t FRAM_pushData(uint16_t page, size_t data_size, void *data);

