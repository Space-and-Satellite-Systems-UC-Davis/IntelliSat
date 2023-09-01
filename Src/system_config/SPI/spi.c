/*
 * spi.c
 *
 *  - August 28-31, 2023 (Creation)
 *      Author	: Darsh
 *      Log		: SPI 1 config, disable, transmitrecieve
 */

#include "spi.h"

void spi1_disable() {
	while(SPI1->SR & SPI_SR_FTLVL);	// Wait till there is no data to transmit
	while(SPI1->SR & SPI_SR_BSY);	// Wait till last data frame is processed
	spi1_end_communication();
	SPI1->SR &= ~SPI_SR_SPE;		// Disable SPI1

	uint8_t temp;
	while(SPI1->SR & SPI_SR_FRLVL){
		// Wait till all data is received
		temp = SPI1->DR;
	}
}

void spi1_config() {
	spi1_disable();

	SPI1->CR1 = 0;
	SPI1->CR2 = 0;
	SPI1->CR1 |=
		  5U << SPI_CR1_BR_Pos		// Baud Rate of `Clock_Source/64`
		| SPI_CR1_SSM				// Software Slave Management (CS is controlled by software)
		| SPI_CR1_MSTR;				// Master Mode
	SPI1->CR2 |=
		  SPI_CR2_FRTXH				// RXNE event generated when RXFIFO is 8 bits full
		| 7U << SPI_CR2_DS_Pos;		// SPI transfer data length is 8 bits

	spi1_enable();

	// Reconsider Baud Rate?
}

// assumes spi1_start_communication() has already been called
bool spi1_transmit_recieve(uint8_t trasnmission, uint8_t *reception) {
	// check for issues...

	// empty out the RXFIFO
	while (SPI1->SR & SPI_SR_RXNE) {
		*reception = SPI1->DR;
	}

	while(!(SPI1->SR & SPI_SR_TXE));					// wait for TXFIFO to be empty
	*((volatile uint8_t) &(SPI1->DR)) = transmission;	// fill TXFIFO with the instruction
	while(!(SPI1->SR & SPI_SR_TXE));					// wait for the instruction to be shifted out

	// wait to receive data if necessary
	if (*reception != NO_RECEPTION_BYTE) {
		while (SPI1->SR & SPI_SR_RXNE) {
			*reception = SPI1->DR;
		}
	}

	return true;
}
