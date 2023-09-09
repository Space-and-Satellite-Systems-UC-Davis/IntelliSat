/*
 * spi.c
 *	- September 5-6, 2023
 *		Author	: Darsh
 *		Log		: SPI 2 config, transmitrecieve. Made generic enable and disable
 *
 *  - August 28-31, 2023 (Creation)
 *      Author	: Darsh
 *      Log		: SPI 1 config, disable, transmitrecieve
 */

#include "spi.h"

void spi_start_communication(GPIO_TypeDef *cs_port, int cs_pin) {
	gpio_low(cs_port, cs_pin);
}

void spi_stop_communication(GPIO_TypeDef *cs_port, int cs_pin) {
	gpio_high(cs_port, cs_pin);
}

void spi_disable(SPI_TypeDef *spi, GPIO_TypeDef *cs_port, int cs_pin) {
	while(spi->SR & SPI_SR_FTLVL);	// Wait till there is no data to transmit
	while(spi->SR & SPI_SR_BSY);	// Wait till last data frame is processed
	spi_stop_communication(cs_port, cs_pin);
	spi->CR1 &= ~SPI_CR1_SPE;		// Disable SPI2

	uint8_t temp;
	while(spi->SR & SPI_SR_FRLVL){
		// Wait till all data is received
		temp = SPI2->DR;
	}
}

void spi2_config() {
	spi_disable(SPI2, SPI2_CS);

	SPI2->CR1 = 0;
	SPI2->CR2 = 0;
	SPI2->CR1 |=
		  5U << SPI_CR1_BR_Pos		// Baud Rate of `Clock_Source/64`
		| SPI_CR1_SSM				// Software Slave Management (CS is controlled by software)
		| SPI_CR1_SSI				// Software Slave Select     (CS is controlled by software)
		| SPI_CR1_MSTR;				// Master Mode
	SPI2->CR2 |=
		  SPI_CR2_FRXTH				// RXNE event generated when RXFIFO is 8 bits full
		| 7U << SPI_CR2_DS_Pos;		// SPI transfer data length is 8 bits

	spi_enable(SPI2);
}

// assumes spi_start_communication(SPI2) has already been called
bool spi2_transmit_recieve(uint8_t* transmission, uint8_t *reception, uint16_t size) {
	// check for issues...


	for (uint16_t i = 0; i < size; i++) {
		while(!(SPI2->SR & SPI_SR_TXE));	// wait for TXFIFO to be empty
		SPI2->DR = transmission[i];			// fill TXFIFO with the instruction

		while (SPI2->SR & SPI_SR_RXNE) {
			reception[i] = SPI2->DR;
		}
	}

	while((SPI2->SR & SPI_SR_BSY));			// wait till all the communication is over

	return true;
}
