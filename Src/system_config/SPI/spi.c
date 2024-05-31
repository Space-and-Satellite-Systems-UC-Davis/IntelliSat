/*
 * spi.c
 *
 *  - October 29, 2023
 *		Author	: Darsh
 *		Log		: Generic spi functions.
 *
 * 	- September 22, 2023
 *		Author	: Darsh
 *		Log		: Included all stages of initializations in spi.h / spi.c
 *
 *	- September 5-6, 2023
 *		Author	: Darsh
 *		Log		: SPI 2 config, transmitrecieve. Made generic enable and disable
 *
 *  - August 28-31, 2023 (Creation)
 *      Author	: Darsh
 *      Log		: SPI 1 config, disable, transmitrecieve
 */

#include "spi.h"

/**
 * Configures GPIO for the SPI-2 Peripheral
 */
void spi3_gpioInit() {

#if OP_REV == 2
	// GPIO
	/* OP R2 GPIO pinout
	 * 		SPI3 SCK		B3		(Alternate Function, AF6)
	 * 		SPI3 MISO		B4		(Alternate Function, AF6)
	 * 		SPI3 MOSI		B5		(Alternate Function, AF6)
	 * 		SPI3 NCS		G15		(Output)
	 */
	// Reset mode on each SPI-3 pin
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOGEN;

	while (GPIOB->OTYPER == 0xFFFFFFFF);
	while (GPIOG->OTYPER == 0xFFFFFFFF);

	GPIOG->PUPDR |= GPIO_PUPDR_PUPD15_0;

	GPIOB->MODER &= ~(
		  GPIO_MODER_MODE3_Msk
		| GPIO_MODER_MODE4_Msk
		| GPIO_MODER_MODE5_Msk);
	GPIOG->MODER &= ~(GPIO_MODER_MODE15_Msk);

	// set each pin to Alternate function, except CS
	GPIOB->MODER |=
		  GPIO_MODER_MODE3_1
		| GPIO_MODER_MODE4_1
		| GPIO_MODER_MODE5_1;
	GPIOG->MODER |= GPIO_MODER_MODE15_0;

	// Reset alternate function selection on each SPI-3 pin
	GPIOB->AFR[0] &= ~(
		  GPIO_AFRL_AFSEL3_Msk
		| GPIO_AFRL_AFSEL4_Msk
		| GPIO_AFRL_AFSEL5_Msk);

	// set each pin to AF5
	GPIOB->AFR[0] |=
		  6U << GPIO_AFRL_AFSEL3_Pos
		| 6U << GPIO_AFRL_AFSEL4_Pos
		| 6U << GPIO_AFRL_AFSEL5_Pos;

#endif

}


void spi2_gpioInit() {

#if OP_REV == 1

	// Reset mode on each SPI-2 pin
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
	while (GPIOB->OTYPER == 0xFFFFFFFF);
	GPIOB->PUPDR |= GPIO_PUPDR_PUPD12_0;
	GPIOB->MODER &= ~(
		  GPIO_MODER_MODE12_Msk
		| GPIO_MODER_MODE13_Msk
		| GPIO_MODER_MODE14_Msk
		| GPIO_MODER_MODE15_Msk);
	// set each pin to Alternate function
	GPIOB->MODER |=
		  GPIO_MODER_MODE12_0
		| GPIO_MODER_MODE13_1
		| GPIO_MODER_MODE14_1
		| GPIO_MODER_MODE15_1;
	// Reset alternate function selection on each SPI-2 pin
	GPIOB->AFR[1] &= ~(
		  GPIO_AFRH_AFSEL13_Msk
		| GPIO_AFRH_AFSEL14_Msk
		| GPIO_AFRH_AFSEL15_Msk);
	// set each pin to AF5
	GPIOB->AFR[1] |=
		  5U << GPIO_AFRH_AFSEL13_Pos
		| 5U << GPIO_AFRH_AFSEL14_Pos
		| 5U << GPIO_AFRH_AFSEL15_Pos;

#endif

}

void spi1_gpioInit() {

#if OP_REV == 2

#endif

}

/**************************** SPI INITIALIZATIONS ****************************/

void spi_disable(SPI_TypeDef *spi, GPIO_TypeDef *cs_port, int cs_pin) {
	while(spi->SR & SPI_SR_FTLVL);	// Wait till there is no data to transmit
	while(spi->SR & SPI_SR_BSY);	// Wait till last data frame is processed
	spi_stopCommunication(cs_port, cs_pin);
	spi->CR1 &= ~SPI_CR1_SPE;		// Disable SPI2

	uint8_t temp;
	while(spi->SR & SPI_SR_FRLVL){
		// Wait till all data is received
		temp = SPI2->DR;
	}
}

void spi1_config() {
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;	// GPIO
	spi1_gpioInit();

	spi_disable(SPI1, SPI1_CS);

	SPI1->CR1 = 0;
	SPI1->CR2 = 0;
	// CR1
	// CR2

	spi_enable(SPI1);

}

void spi2_config() {
	RCC->APB1ENR1 |= RCC_APB1ENR1_SPI2EN;	// Clock
	spi2_gpioInit();						// GPIO

	spi_disable(SPI2, SPI2_CS);

	SPI2->CR1 = 0;
	SPI2->CR2 = 0;
	SPI2->CR1 |=
		  5U << SPI_CR1_BR_Pos		// Baud Rate of `Clock_Source/64` (78.125 KHz)
		| SPI_CR1_SSM				// (CS is controlled by software)
		| SPI_CR1_SSI				// (CS is controlled by software)
		| SPI_CR1_MSTR;
	SPI2->CR2 |=
		  SPI_CR2_FRXTH			// RXNE generated when RXFIFO has 1 byte
		| 7U << SPI_CR2_DS_Pos;	// Transfer Data Length is 1 Byte

	spi_enable(SPI2);
}

void spi3_config() {
	RCC->APB1ENR1 |= RCC_APB1ENR1_SPI3EN;	// Clock
    spi3_gpioInit();						// GPIO

	spi_disable(SPI3, SPI3_CS);

	SPI3->CR1 = 0;
	SPI3->CR2 = 0;
	SPI3->CR1 |=
		  0U << SPI_CR1_BR_Pos		// Baud Rate of `Clock_Source/2` (2.5 MHz)
		| SPI_CR1_SSM				// (CS is controlled by software)
		| SPI_CR1_SSI				// (CS is controlled by software)
		| SPI_CR1_MSTR
		| SPI_CR1_CPOL				// Clock line will be HIGH when IDLE
		| SPI_CR1_CPHA;				// Clock transitions happen with Data Transitions
	SPI3->CR2 |=
		  SPI_CR2_FRXTH			// RXNE generated when RXFIFO has 1 byte
		| 7U << SPI_CR2_DS_Pos;	// Transfer Data Length is 1 Byte

	spi_enable(SPI3);
}

void spi_config(SPI_TypeDef *spi) {
	switch ((uint32_t)spi) {
		case (uint32_t)SPI1:
			spi1_config();
			break;
		case (uint32_t)SPI2:
			spi2_config();
			break;
		case (uint32_t)SPI3:
			spi3_config();
			break;
	}
}

/***************************** SPI COMMUNICATION *****************************/
void spi_startCommunication(GPIO_TypeDef *cs_port, int cs_pin) {

	gpio_low(cs_port, cs_pin);
}void spi_stopCommunication(GPIO_TypeDef *cs_port, int cs_pin) {
	gpio_high(cs_port, cs_pin);
}

bool spi_transmitReceive(SPI_TypeDef* spi, uint8_t* transmission, uint8_t *reception, uint16_t size, bool dma) {
	while (size-- >= 1) {
		// wait for TXFIFO to be empty
		while(!(spi->SR & SPI_SR_TXE));	// TXE = TX Empty
		if (transmission == NULL) {
			// send a dummy byte to trigger the clock pulses
			*((volatile uint8_t*) &(spi->DR)) = SPI_DUMMY_BYTE;
		} else {
			// fill TXFIFO with the instruction
			*((volatile uint8_t*) &(spi->DR)) = *transmission;
			transmission++;
		}
		while(!(spi->SR & SPI_SR_TXE));

		// read the reception line until it's empty
		while (spi->SR & SPI_SR_RXNE) {	// RXNE = RX Not Empty
			if (reception == NULL) {
				spi->DR;
			} else {
				*reception = spi->DR;
				reception++;
			}
		}
	}
	return true;
}
