/*
 * qspi.h
 *
 *  - October 22, 2023
 *      Author: Darsh
 *      Log   : Finally adding everything to the qspi.h file
 */

#ifndef REALOP1_QSPI_H_
#define REALOP1_QSPI_H_

#include "stm32l476xx.h"
#include <globals.h>

#define QSPI_TIMEOUT_PERIOD  	1000000	// TODO

// Macros for FLASH Instruction mode (fmode in qspi_set_command())
#define QSPI_FMODE_INDIRECT_WRITE		0x00
#define QSPI_FMODE_INDIRECT_READ		0x01
#define QSPI_FMODE_AUTOMATIC_POLLING	0x02
#define QSPI_FMODE_MEMORY_MAPPED		0x03

#define QSPI_READ	false
#define QSPI_WRITE	true

// Macros for # of wires per QSPI Instruction Phase (in qspi_set_command())
#define QSPI_UNUSED	0x00
#define QSPI_1_WIRE	0x01
#define QSPI_2_WIRE	0x02
#define QSPI_4_WIRE	0x03

#define qspi_on()	   RCC->AHB3ENR |= RCC_AHB3ENR_QSPIEN
#define qspi_off()	   RCC->AHB3ENR &= ~RCC_AHB3ENR_QSPIEN;
#define qspi_enable()  QUADSPI->CR |=   QUADSPI_CR_EN
#define qspi_disable() QUADSPI->CR &= ~(QUADSPI_CR_EN)

/**
 * Configures the QSPI hardware so that it can be used later
 * 
 * @param flash_size      An integer that obeys : FLASH_SIZE_BITS = 2^(flash_size-1)
 * @param address_size    # of Bytes used by an address in the FLASH
 * @param alternateb_size # of Bytes used by an Alternate Bytes
 */
void qspi_config(
		uint8_t flash_size,
		uint8_t address_size,
		uint8_t alternateb_size
);

/**
 * Sets up a QSPI communication
 * Must be run before the other qspi_start_command() and qspi_status_polling()
 *
 * @param fmode	  The Instruction Type
 * @param imode	  The # of wires used during Instruction Phase
 * @param admode  The # of wires used during Address Phase
 * @param abmode  The # of wires used during Alternate Bytes Phase
 * @param dcyc	  The # of dummy cycles needed
 * @param dmode   The # of wires used during Data Phase
 * @param dma	  Whether DMA should be used or not
 *
 * @returns Whether the communication was set up or not
 */
bool qspi_setCommand(
		uint8_t fmode,
		uint8_t imode,
		uint8_t admode,
		uint8_t abmode,
		uint8_t dcyc,
		uint8_t dmode,
		bool    dma
);

/**
 * Starts a QSPI communication
 * Assumes that qspi_set_command() is already run
 *
 * @param instruction    The instruction to send
 * @param address        The address in the external memory
 * @param data_length    The amount of data (# of bytes) in the data phase
 * @param data			 The buffer where data is located / will be stored
 * @param r_or_w		 either QSPI_READ or QSPI_WRITE (both are macros)
 * @param timeout_period To trigger a timeout {not implemented}
 *
 * @returns Whether the communication was started properly or not
 */
bool qspi_sendCommand(
		uint8_t  instruction,
		uint32_t address,
		uint32_t data_length,
		uint8_t  *data,
		bool     r_or_w,
		uint32_t timeout_period
);

/**
 * Starts the Status Polling Mode
 * Assumes that qspi_set_command() is already run
 *
 * @param polling_mode   0:AND , 1:OR
 * @param instruction    instruction to be sent
 * @param mask		     the mask used prior to checking for matches
 * @param match		     the desired pattern
 * @param timeout_period to trigger a timeout {not implemented}
 *
 * @returns none
 */
bool qspi_statusPoll(
		bool polling_mode,
		uint8_t instruction,
		uint8_t mask,
		uint8_t match,
		uint32_t timeout_period
);

#define QSPI_READY            '4'	// qspi_set_command() has been run
#define QSPI_BUSY             '3'	// executing a previous command
#define QSPI_SUCCESSFUL       '2'	// status of the previously executed command
#define QSPI_TIMEDOUT         '1'	// status of the previously executed command
#define QSPI_TRANSFER_ERROR   '0'	// status of the previously executed command

/**
 * Returns the status of the QUADSPI hardware
 *
 * @param none
 * @returns Macros {QSPI_READY, QSPI_BUSY, QSPI_SUCCESSFUL, QSPI_TIMEDOUT, QSPI_TRANSFER_ERROR}
 */
char qspi_getStatus();

#define QSPI_DMA_UNAVAILABLE 	false


#endif
