/**
 * @file flash_spi.h
 *
 * @brief Bit-banged single-line SPI (mode 0) to the W25Q128JV FLASH.
 *
 * The FLASH is wired to the QUADSPI pins, and its clock pin has no SPI
 * alternate function, so the bus is driven from GPIO instead of an SPI
 * peripheral. /WP and /HOLD are held high.
 *
 * @author Li, Chun Ho (lchli@ucdavis.edu)
 * @date 10/24/2026
 */

#ifndef REALOP1_PERIPHERALS_FLASH_SPI_H_
#define REALOP1_PERIPHERALS_FLASH_SPI_H_

#include "stm32l476xx.h"
#include <globals.h>

/**
 * Configures the FLASH pins as GPIO with chip select released.
 * Must run before any flashSpi_command().
 */
void flashSpi_init();

/**
 * Runs one command: instruction byte, optional 24-bit address, then len data
 * bytes. Pass tx to send data (program, register write), rx to receive it
 * (read, register read), or both NULL with len 0 for no data phase.
 *
 * MISO is pulled up, so a missing chip reads 0xFF: busy, never ready.
 */
void flashSpi_command(uint8_t instruction, bool hasAddress, uint32_t address,
                      const uint8_t *tx, uint8_t *rx, uint32_t len);

#endif /* REALOP1_PERIPHERALS_FLASH_SPI_H_ */
