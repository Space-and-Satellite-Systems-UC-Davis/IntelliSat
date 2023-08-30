/*
 * spi.c
 *
 *  - August 28, 2023 (Creation)
 *      Author	: Darsh
 *      Log		: ...
 */

#include "spi.h"

#define SPI1_CS           GPIOE,12
#define NO_RECEPTION_BYTE 0xA1


void spi1_start_communication() {
	gpio_low(SPI1_CS);
}

void spi1_end_communication() {
	gpio_high(SPI1_CS);
}

void spi1_disable() {
	while(SPI1->SR & SPI_SR_FTLVL);	// Wait till there is no data to transmit
	while(SPI1->SR & SPI_SR_BSY);	// Wait till last data frame is processed
	spi1_end_communication();
	SPI1->SR &= ~SPI_SR_SPE;		// Disable SPI1
	while(SPI1->SR & SPI_SR_FRLVL){
		// Wait till there is all data is received
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
bool spi1_transmit_recieve(uint8_t trasnmission, uint8_t *reception, uint16_t size) {
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
		while (SPI1->SR & SPI_SR_RXNE);		// wait for RXFIFO to get to recieved data
		*reception = SPI1->DR;				// read from the RXFIFO
	}

	return true;

	// should I move the initial RXFIFO emptying loop into the case where we read? So that it's only going into the loop if the caller cares about RXFIFO?
}
