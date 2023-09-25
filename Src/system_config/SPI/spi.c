/*
 * spi.c
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

/**************************** SPI INITIALIZATIONS ****************************/

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

void spi1_config() {
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;	// Clock

	// GPIO

	spi_disable(SPI1, SPI1_CS);

	SPI1->CR1 = 0;
	SPI1->CR2 = 0;
	// CR1
	// CR2

	spi_enable(SPI1);

}

void spi2_config() {
	RCC->APB1ENR1 |= RCC_APB1ENR1_SPI2EN;	// Clock

	// Reset mode on each SPI-2 pin
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
	while (GPIOB->OTYPER == 0xFFFFFFFF);
	GPIOB->PUPDR |= SPI2_CS_PUPD;
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

	spi_disable(SPI2, SPI2_CS);

	SPI2->CR1 = 0;
	SPI2->CR2 = 0;
	SPI2->CR1 |=
		  5U << SPI_CR1_BR_Pos		// Baud Rate of `Clock_Source/64`
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

	// GPIO

	// spi_disable(SPI3, SPI3_CS);

	SPI3->CR1 = 0;
	SPI3->CR2 = 0;
	// CR1
	// CR2

	spi_enable(SPI3);

}

/***************************** SPI COMMUNICATION *****************************/

void spi_start_communication(GPIO_TypeDef *cs_port, int cs_pin) {
	gpio_low(cs_port, cs_pin);
}

void spi_stop_communication(GPIO_TypeDef *cs_port, int cs_pin) {
	gpio_high(cs_port, cs_pin);
}

/*
bool spi1_transmit_recieve(uint8_t* transmission, uint8_t *reception, uint16_t size) {

}
*/


// assumes spi_start_communication(SPI2) has already been called
bool spi2_transmit_recieve(uint8_t* transmission, uint8_t *reception, uint16_t size) {
	// check for issues...

	while(size-- > 1) {
		while(!(SPI2->SR & SPI_SR_TXE));	// wait for TXFIFO to be empty
		SPI2->DR = (*transmission)++;			// fill TXFIFO with the instruction

		if (reception) {
			while (SPI2->SR & SPI_SR_RXNE) {
				*(reception++) = SPI2->DR;
			}
		}
	}
	while((SPI2->SR & SPI_SR_BSY));			// wait till all the communication is over

	// DMA1_Channel 4 = RX

//	DMA1_Channel4->CPAR  = SPI2->DR;
//	DMA1_Channel4->CMAR  = transmission;
//	DMA1_Channel4->CNDTR = size;
//	DMA1_Channel4->CCR  |= DMA_CCR_EN;

	// DMA1_Channel 5 = TX

	return true;
}

/*
bool spi3_transmit_recieve(uint8_t* transmission, uint8_t *reception, uint16_t size) {
	return true;
}
*/
