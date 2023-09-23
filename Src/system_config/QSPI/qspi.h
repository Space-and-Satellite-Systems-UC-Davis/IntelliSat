/*
 *
 */

#ifndef REALOP1_QSPI_H_
#define REALOP1_QSPI_H_

#include "stm32l476xx.h"
#include "../../globals.h"

#define QSPI_BUSY             1
#define QSPI_SUCCESSFUL       0
#define QSPI_TIMEDOUT        -1
#define QSPI_TRANSFER_ERROR  -2


#define qspi_on()	   RCC->AHB3ENR |= RCC_AHB3ENR_QSPIEN
#define qspi_off()	   RCC->AHB3ENR &= ~RCC_AHB3ENR_QSPIEN;
#define qspi_enable()  QUADSPI->CR |=   QUADSPI_CR_EN
#define qspi_disable() QUADSPI->CR &= ~(QUADSPI_CR_EN)

void qspi_config(uint8_t flash_size);

bool qspi_set_command(
		uint8_t fmode,
		uint8_t imode,
		uint8_t admode,
		uint8_t abmode,
		uint8_t dcyc,
		uint8_t dmode);

bool qspi_send_command(
		uint8_t instruction,
		uint32_t address,
		uint32_t data_length,
		uint8_t *data,
		bool r_or_w,
		uint32_t timeout_period);

bool qspi_status_poll(
		uint8_t instruction,
		uint8_t mask,
		uint8_t match,
		uint32_t timeout_period);

#endif
