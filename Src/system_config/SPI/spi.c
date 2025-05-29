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
 *		Log		: SPI 2 config, transmitreceive. Made generic enable and disable
 *
 *  - August 28-31, 2023 (Creation)
 *      Author	: Darsh
 *      Log		: SPI 1 config, disable, transmitreceive
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

	wait_with_timeout(is_GPIOB_not_ready, DEFAULT_TIMEOUT_MS);
	wait_with_timeout(is_GPIOG_not_ready, DEFAULT_TIMEOUT_MS);


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
#elif OP_REV == 3
// GPIO
	/* OP R3 GPIO pinout
	 * 		SPI3 SCK		B3		(Alternate Function, AF6)
	 * 		SPI3 MISO		B4		(Alternate Function, AF6)
	 * 		SPI3 MOSI		B5		(Alternate Function, AF6)
	 * 		SPI3 NCS		B6		(Output)
	 */
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
	wait_with_timeout(is_GPIOB_not_ready, DEFAULT_TIMEOUT_MS);

	GPIOB->PUPDR &= ~GPIO_PUPDR_PUPD4_0;

	GPIOB->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED3_Msk;


	GPIOB->MODER &= ~(
		  GPIO_MODER_MODE3_Msk
		| GPIO_MODER_MODE4_Msk
		| GPIO_MODER_MODE5_Msk
		| GPIO_MODER_MODE6_Msk);

	// set each pin to Alternate function, except CS
	GPIOB->MODER |=
		  GPIO_MODER_MODE3_1
		| GPIO_MODER_MODE4_1
		| GPIO_MODER_MODE5_1;
	GPIOB->MODER |= GPIO_MODER_MODE6_0;

	// Reset alternate function selection on each SPI-3 pin
	GPIOB->AFR[0] &= ~(
		  GPIO_AFRL_AFSEL3_Msk
		| GPIO_AFRL_AFSEL4_Msk
		| GPIO_AFRL_AFSEL5_Msk);

	// set each pin to AF6
	GPIOB->AFR[0] |=
		  GPIO_AFRH_AFSEL_AF6 << GPIO_AFRL_AFSEL3_Pos
		| GPIO_AFRH_AFSEL_AF6 << GPIO_AFRL_AFSEL4_Pos
		| GPIO_AFRH_AFSEL_AF6 << GPIO_AFRL_AFSEL5_Pos;
#endif

}


void spi2_gpioInit() {

#if OP_REV == 1 || OP_REV == 3
	/* OP R3 GPIO pinout
	 * 		SPI2 SCK		B13		(Alternate Function, AF5)
	 * 		SPI2 MISO		B14		(Alternate Function, AF5)
	 * 		SPI2 MOSI		B15		(Alternate Function, AF5)
	 * 		SPI2 NCS		B12		(Output)
	 */
	// Reset mode on each SPI-2 pin
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;

	wait_with_timeout(is_GPIOB_not_ready, DEFAULT_TIMEOUT_MS);
	GPIOB->PUPDR &= ~GPIO_PUPDR_PUPD12_0;

	GPIOB->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED13_Msk;

	GPIOB->MODER &= ~(
		  GPIO_MODER_MODE12_Msk
		| GPIO_MODER_MODE13_Msk
		| GPIO_MODER_MODE14_Msk
		| GPIO_MODER_MODE15_Msk);
	// set each pin to Alternate function except CS
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
		  GPIO_AFRH_AFSEL_AF5 << GPIO_AFRH_AFSEL13_Pos
		| GPIO_AFRH_AFSEL_AF5 << GPIO_AFRH_AFSEL14_Pos
		| GPIO_AFRH_AFSEL_AF5 << GPIO_AFRH_AFSEL15_Pos;

#endif

}

void spi1_gpioInit() {

#if OP_REV == 2

#endif

}

/**************************** SPI INITIALIZATIONS ****************************/

void spi_dma_disable(SPI_TypeDef *spi) {
	switch ((uint32_t)spi) {
		case (uint32_t)SPI1:
			dma_disable_channel(SELECT_SPI1_RX);
			dma_disable_channel(SELECT_SPI1_TX);
			break;
		case (uint32_t)SPI2:
			dma_disable_channel(SELECT_SPI2_RX);
			dma_disable_channel(SELECT_SPI2_TX);
			break;
		case (uint32_t)SPI3:
			dma_disable_channel(SELECT_SPI3_RX);
			dma_disable_channel(SELECT_SPI3_TX);
			break;
	}
}

//The order of dma_enable_channel matters
void spi_dma_enable_rx(SPI_TypeDef *spi) {
	switch ((uint32_t)spi) {
		case (uint32_t)SPI1:
			spi_disable(SPI1, SPI1_CS);
			SPI1->CR2 |= SPI_CR2_RXDMAEN;
			dma_enable_channel(SELECT_SPI1_RX);

			break;
		case (uint32_t)SPI2:
			spi_disable(SPI2, SPI2_CS);
			SPI2->CR2 |= SPI_CR2_RXDMAEN;
			dma_enable_channel(SELECT_SPI2_RX);

			break;
		case (uint32_t)SPI3:
			spi_disable(SPI3, SPI3_CS);
			SPI3->CR2 |= SPI_CR2_RXDMAEN;
			dma_enable_channel(SELECT_SPI3_RX);

			break;
	}

	spi_enable(spi);
}
//The order of dma_enable_channel matters
void spi_dma_enable_tx(SPI_TypeDef *spi) {
	switch ((uint32_t)spi) {
		case (uint32_t)SPI1:
			spi_disable(SPI1, SPI1_CS);
			dma_enable_channel(SELECT_SPI1_TX);
			SPI1->CR2 |= SPI_CR2_TXDMAEN;
			break;
		case (uint32_t)SPI2:
			spi_disable(SPI2, SPI2_CS);
			dma_enable_channel(SELECT_SPI2_TX);
			SPI2->CR2 |= SPI_CR2_TXDMAEN;
			break;
		case (uint32_t)SPI3:
			spi_disable(SPI3, SPI3_CS);
			dma_enable_channel(SELECT_SPI3_TX);
			SPI3->CR2 |= SPI_CR2_TXDMAEN;
			break;
	}

	spi_enable(spi);
}

void spi_dma_configure(SPI_TypeDef *spi, uint8_t rx_buffer[], uint8_t tx_buffer[], uint16_t size) {

    DMAConfig rx_config;
    DMAConfig tx_config;

	switch ((uint32_t)spi) {
		case (uint32_t)SPI1:
			rx_config.selection = SELECT_SPI1_RX;
			tx_config.selection = SELECT_SPI1_TX;
			break;
		case (uint32_t)SPI2:
			rx_config.selection = SELECT_SPI2_RX;
			tx_config.selection = SELECT_SPI2_TX;
			break;
		case (uint32_t)SPI3:
			rx_config.selection = SELECT_SPI3_RX;
			tx_config.selection = SELECT_SPI3_TX;
			break;
	}

	rx_config.length = size;
	rx_config.memory_addr = (uint32_t)rx_buffer;
    rx_config.peripheral_addr = (uint32_t) &(spi->DR);
    rx_config.pdata_size = sizeof(uint8_t);
    rx_config.mdata_size = sizeof(uint8_t);
    rx_config.circular = false;
    rx_config.peripheral_to_memory = true;
    rx_config.peripheral_increment = false;
    rx_config.memory_increment = true;

	tx_config.length = size;
	tx_config.memory_addr = (uint32_t)tx_buffer;
    tx_config.peripheral_addr = (uint32_t) &(spi->DR);
    rx_config.pdata_size = sizeof(uint8_t);
    rx_config.mdata_size = sizeof(uint8_t);
    tx_config.circular = false;
    tx_config.peripheral_to_memory = false;
    tx_config.peripheral_increment = false;
    tx_config.memory_increment = true;

    configure_channel(rx_config);
    configure_channel(tx_config);
}

//To close DMA communication it is mandatory to follow these steps in order:
// 1. Disable DMA streams
// 2. Disable SPI as normal
// 3. Clear the TXDMAEN and RXDMAEN bits
void spi_disable(SPI_TypeDef *spi, GPIO_TypeDef *cs_port, int cs_pin) {
	spi_dma_disable(spi);

	uint64_t start_time = getSysTime(); //time in ms
	while((spi->SR & SPI_SR_FTLVL) && !(is_time_out(start_time, DEFAULT_TIMEOUT_MS)));	// Wait till there is no data to transmit

	start_time = getSysTime();
	while((spi->SR & SPI_SR_BSY) && !(is_time_out(start_time, DEFAULT_TIMEOUT_MS)));	// Wait till last data frame is processed

	spi_stopCommunication(cs_port, cs_pin);
	spi->CR1 &= ~SPI_CR1_SPE;	

	spi->CR2 &= ~(SPI_CR2_RXDMAEN | SPI_CR2_TXDMAEN); //Disable the DMA flags

	start_time = getSysTime();
	uint8_t temp;
	while(spi->SR & SPI_SR_FRLVL && !(is_time_out(start_time, DEFAULT_TIMEOUT_MS))){
		// Wait till all data is received
		temp = spi->DR;
	}
}

void spi1_config() {
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;	// GPIO
	spi1_gpioInit();

	spi_disable(SPI1, SPI1_CS);

	SPI1->CR1 = SPI_CR_RESET;
	SPI1->CR2 = SPI_CR_RESET;
	// CR1
	// CR2

	spi_enable(SPI1);

}

void spi2_config() {
	RCC->APB1ENR1 |= RCC_APB1ENR1_SPI2EN;	// Clock
	spi2_gpioInit();						// GPIO

	spi_disable(SPI2, SPI2_CS);

	SPI2->CR1 = SPI_CR_RESET;
	SPI2->CR2 = SPI_CR_RESET;
	SPI2->CR1 |=
		  SPI_CR1_BR_R2 << SPI_CR1_BR_Pos		// Baud Rate of `Clock_Source/2` (2.5 MHz)
		| SPI_CR1_SSM				// (CS is controlled by software)
		| SPI_CR1_SSI				// (CS is controlled by software)
		| SPI_CR1_MSTR
		| SPI_CR1_CPOL
		| SPI_CR1_CPHA;	

	SPI2->CR2 |=
		  SPI_CR2_FRXTH			// RXNE generated when RXFIFO has 1 byte
		| SPI_CR2_DS_8_BIT << SPI_CR2_DS_Pos;	// Transfer Data Length is 1 Byte
  
  SPI3->CR1 &= ~(SPI_CR1_BIDIMODE | SPI_CR1_RXONLY);
	spi_enable(SPI2);
}

void spi3_config() {
	RCC->APB1ENR1 |= RCC_APB1ENR1_SPI3EN;	// Clock
    spi3_gpioInit();						// GPIO

	spi_disable(SPI3, SPI3_CS);

	SPI3->CR1 = SPI_CR_RESET;
	SPI3->CR2 = SPI_CR_RESET;
	SPI3->CR1 |=
		  SPI_CR1_BR_R2 << SPI_CR1_BR_Pos		// Baud Rate of `Clock_Source/2` (2.5 MHz)
		| SPI_CR1_SSM				// (CS is controlled by software)
		| SPI_CR1_SSI				// (CS is controlled by software)
		| SPI_CR1_MSTR
		| SPI_CR1_CPOL
		| SPI_CR1_CPHA;				
	SPI3->CR1 &= ~(SPI_CR1_BIDIMODE | SPI_CR1_RXONLY);

	SPI3->CR2 |=
		  SPI_CR2_FRXTH			// RXNE generated when RXFIFO has 1 byte
		| SPI_CR2_DS_8_BIT << SPI_CR2_DS_Pos;	// Transfer Data Length is 1 Byte

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
}
void spi_stopCommunication(GPIO_TypeDef *cs_port, int cs_pin) {
	gpio_high(cs_port, cs_pin);
}

bool spi_transmitReceive(SPI_TypeDef* spi, uint8_t* transmission, uint8_t *reception, uint16_t size, bool dma) {
	uint64_t start_time = getSysTime(); //time in ms
	while ((size-- >= 1) && !(is_time_out(start_time, DEFAULT_TIMEOUT_MS))) {
		uint64_t inner_start_time = getSysTime();

		// wait for TXFIFO to be empty
		while(!(spi->SR & SPI_SR_TXE) && !(is_time_out(inner_start_time, DEFAULT_TIMEOUT_MS)));	// TXE = TX Empty
		if (transmission == NULL) {
			// send a dummy byte to trigger the clock pulses
			*((volatile uint8_t*) &(spi->DR)) = SPI_DUMMY_BYTE;
		} else {
			// fill TXFIFO with the instruction
			*((volatile uint8_t*) &(spi->DR)) = *transmission;
			transmission++;
		}

    
		inner_start_time = getSysTime();
		while(!(spi->SR & SPI_SR_TXE) && !(is_time_out(inner_start_time, DEFAULT_TIMEOUT_MS)));
		inner_start_time = getSysTime();
		while(!(spi->SR & SPI_SR_RXNE) && !(is_time_out(inner_start_time, DEFAULT_TIMEOUT_MS)));

		// read the reception line until it's empty
		inner_start_time = getSysTime();
		while ((spi->SR & SPI_SR_RXNE) && !(is_time_out(inner_start_time, DEFAULT_TIMEOUT_MS))) {	// RXNE = RX Not Empty
			if (reception == NULL) {
				spi->DR;
			} else {
				uint16_t value = spi->DR;
				*reception = value;
				reception++;
			}
		}
	}
	return true;
}
