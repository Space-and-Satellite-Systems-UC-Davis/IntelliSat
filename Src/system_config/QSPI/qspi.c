/*
 *
 */

#include "qspi.h"

short int qspi_status;
bool qspi_in_use = false;
bool qspi_dma_use = false;
bool timeout = false;

/*
 *
 */
int get_qspi_status() {
	return qspi_status;
}

/*
 *
 */
void qspi_config(uint8_t flash_size) {
	qspi_on();	// qspi peripheral clock

	// GPIO
	/* OP R1 GPIO pinout
	 * 		QSPI CS			E11		(Alternate Function, AF10)
	 * 		QSPI CLK		E10		(Alternate Function, AF10)
	 * 		QSPI IO0		B1		(Alternate Function, AF10)
	 * 		QSPI IO1		B0		(Alternate Function, AF10)
	 * 		QSPI IO2		E14		(Alternate Function, AF10)
	 * 		QSPI IO3		E15		(Alternate Function, AF10)
	 */
	// Reset mode on each QUADSPI pin
	GPIOB->MODER &= ~(
		  GPIO_MODER_MODE0_Msk
		| GPIO_MODER_MODE1_Msk);
	GPIOE->MODER &= ~(
			  GPIO_MODER_MODE10_Msk
			| GPIO_MODER_MODE11_Msk
			| GPIO_MODER_MODE14_Msk
			| GPIO_MODER_MODE15_Msk);
	// set each pin to Alternate function
	GPIOB->MODER |=
		  GPIO_MODER_MODE0_1
		| GPIO_MODER_MODE1_1;
	GPIOE->MODER |=
		  GPIO_MODER_MODE10_1
		| GPIO_MODER_MODE11_1
		| GPIO_MODER_MODE14_1
		| GPIO_MODER_MODE15_1;
	// Reset alternate function selection on QUADSPI
	GPIOB->AFR[0] &= ~(
		  GPIO_AFRL_AFSEL0_Msk |
		  GPIO_AFRL_AFSEL1_Msk);
	GPIOE->AFR[1] &= ~(
			  GPIO_AFRH_AFSEL10_Msk |
			  GPIO_AFRH_AFSEL11_Msk |
			  GPIO_AFRH_AFSEL14_Msk |
			  GPIO_AFRH_AFSEL15_Msk);
	// set each pin to AF10
	GPIOB->AFR[0] |=
		  10U << GPIO_AFRL_AFSEL0_Pos |
		  10U << GPIO_AFRL_AFSEL1_Pos;
	GPIOE->AFR[1] |=
		  10U << GPIO_AFRH_AFSEL10_Pos |
		  10U << GPIO_AFRH_AFSEL11_Pos |
		  10U << GPIO_AFRH_AFSEL14_Pos |
		  10U << GPIO_AFRH_AFSEL15_Pos;

	qspi_disable();
	QUADSPI->CR = 0;
	QUADSPI->CR |=
			(79U << QUADSPI_CR_PRESCALER_Pos) |
			QUADSPI_CR_SSHIFT;

	QUADSPI->DCR = 0;
	QUADSPI->DCR |= (flash_size << QUADSPI_DCR_FSIZE_Pos);
	qspi_enable();
}

/*
 *
 */
bool qspi_set_command(
		uint8_t fmode,
		uint8_t imode,
		uint8_t admode,
		uint8_t abmode,
		uint8_t dcyc,
		uint8_t dmode,
		bool    dma
) {
	if (qspi_status == QSPI_BUSY || (dma && QSPI_DMA_UNAVAILABLE)) {
		return false;
	}
	qspi_disable();
	QUADSPI->CR &= ~QUADSPI_CR_TCIE;
	QUADSPI->CCR &=
		  ~(QUADSPI_CCR_FMODE)
		& ~(QUADSPI_CCR_IMODE)
		& ~(QUADSPI_CCR_ADMODE)
		& ~(QUADSPI_CCR_ABMODE)
		& ~(QUADSPI_CCR_DCYC)
		& ~(QUADSPI_CCR_DMODE);
	QUADSPI->CCR &= ~QUADSPI_CCR_INSTRUCTION_Msk;
	QUADSPI->CCR |=
		  (fmode << QUADSPI_CCR_FMODE_Pos)
		| (imode << QUADSPI_CCR_IMODE_Pos)
		| (admode << QUADSPI_CCR_ADMODE_Pos)
		| (abmode << QUADSPI_CCR_ABMODE_Pos)
		| (dcyc << QUADSPI_CCR_DCYC_Pos)
		| (dmode << QUADSPI_CCR_DMODE_Pos);
	if (dma) {
		DMA2_Channel7->CCR &= ~(
			  DMA_CCR_MEM2MEM_Msk
			| DMA_CCR_MSIZE_Msk
			| DMA_CCR_PSIZE_Msk
			| DMA_CCR_MINC_Msk
			| DMA_CCR_PINC_Msk
			| DMA_CCR_CIRC_Msk
			| DMA_CCR_DIR_Msk
			| DMA_CCR_EN_Msk
		);
		RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
		DMA2_CSELR->CSELR &= ~(DMA_CSELR_C7S_Msk);
		DMA2_CSELR->CSELR |= (3U << DMA_CSELR_C7S_Pos);
		DMA2_Channel7->CCR |=
			  (0x00 << DMA_CCR_MSIZE_Pos)
			| (0x00 << DMA_CCR_PSIZE_Pos)
			| DMA_CCR_MINC;
	}
	qspi_status = QSPI_READY;
	return true;
}

/*
 *
 */
bool qspi_send_command(
		uint8_t instruction,
		uint32_t address,
		uint32_t data_length,
		uint8_t *data,
		bool r_or_w,
		uint32_t timeout_period,
		bool dma
) {
	if (qspi_status != QSPI_READY) {
		return false;
	}

	if (dma) {
		DMA2_Channel7->CNDTR = (uint16_t)data_length;
		DMA2_Channel7->CPAR  = QUADSPI->DR;
		DMA2_Channel7->CMAR  = data;
		DMA2_Channel7->CCR  |=
			  (r_or_w << DMA_CCR_DIR)
			| DMA_CCR_EN;
		QUADSPI->CR |= QUADSPI_CR_DMAEN;
	}

	qspi_enable();
	QUADSPI->DLR =  data_length - 1;
	QUADSPI->CCR |= (instruction << QUADSPI_CCR_INSTRUCTION_Pos);
	QUADSPI->AR  =   address;

	qspi_status = QSPI_BUSY;

	if (dma) {
		/*
		 * When the transfer is complete, the TC interrupt of the QSPI
		 * will close the DMA Channel as well as the QSPI process
		 */
		QUADSPI->CR |= QUADSPI_CR_TCIE;
		qspi_dma_use = true;
		return true;
	}

	for (uint64_t i = 0; i < data_length; i++) {
		while (!(QUADSPI->SR & QUADSPI_SR_FTF)) {
			if ((r_or_w == true) && (QUADSPI->SR & QUADSPI_SR_TCF)) {
				break;
			}
			if (qspi_status == QSPI_TIMEDOUT) {	// set by an interrupt
				QUADSPI->CR |= QUADSPI_CR_ABORT;
				goto qspi_send_command_complete;
			}
		}
		if (r_or_w == true) {
			data[i] = *(__IO uint8_t *)((__IO uint32_t *) &QUADSPI->DR);
		} else {
			*(__IO uint8_t *)((__IO uint32_t *) &QUADSPI->DR) = data[i];
		}
	}

qspi_send_command_complete:

	while (QUADSPI->SR & QUADSPI_SR_BUSY);
	qspi_disable();
	if (qspi_status != QSPI_TIMEDOUT) {
		qspi_status = QSPI_SUCCESSFUL;
	}
	return true;
}




// Status-Polling Mode for 4 wire QSPI
bool qspi_status_poll(
		uint8_t instruction,
		uint8_t mask,
		uint8_t match,
		uint32_t timeout_period
) {
	if (qspi_status == QSPI_BUSY) {
		return false;
	}
	qspi_in_use = QSPI_BUSY;

	qspi_disable();
	QUADSPI->PSMKR = mask;
	QUADSPI->PSMAR = match;
	QUADSPI->PIR   = 0x10;	// Polling Interval (# of clocks)
	QUADSPI->CR |=
		  QUADSPI_CR_APMS	// Ends the transaction after the match
		| QUADSPI_CR_SMIE;	// Will Interrupt when the match is found

	QUADSPI->CCR &= ~(
		  QUADSPI_CCR_FMODE_Msk
		| QUADSPI_CCR_DMODE_Msk
		| QUADSPI_CCR_ADMODE_Msk
		| QUADSPI_CCR_IMODE
		| QUADSPI_CCR_INSTRUCTION_Msk);

	qspi_enable();
	QUADSPI->CCR |=
		  (2U << QUADSPI_CCR_FMODE_Pos)
		| (3U << QUADSPI_CCR_DMODE_Pos)
		| (3U << QUADSPI_CCR_ADMODE_Pos)
		| (1U << QUADSPI_CCR_IMODE_Pos)
		| (instruction << QUADSPI_CCR_INSTRUCTION_Pos);

	/*
	 * Once the Instruction is written, QSPI peripheral will keep polling
	 * until the match is found. Then the interrupt is responsible for
	 * clearing the flag, stopping the status polling mode, and stopping
	 * the peripheral
	 * Until then, qspi_status = QSPI_BUSY will indicate that
	 * the peripheral can't be used.
	 */

	return true;
}



#define qspi_stop_status_polling() { \
		QUADSPI->CCR  &= ~(QUADSPI_CCR_FMODE | QUADSPI_CCR_DMODE); \
		QUADSPI->CR   &= ~QUADSPI_CR_APMS; \
}

/*
 *
 */
void QUADSPI_IRQHandler() {
	if (QUADSPI->SR & QUADSPI_SR_SMF) {	// Status Polling complete
		QUADSPI->FCR |= QUADSPI_FCR_CSMF;
		QUADSPI->CR  &= ~QUADSPI_CR_SMIE;

		qspi_stop_status_polling();
		qspi_disable();
		qspi_status = QSPI_SUCCESSFUL;
	}
	if (QUADSPI->SR & QUADSPI_SR_TEF) {	// Transfer Error
		QUADSPI->FCR |= QUADSPI_FCR_CTEF;

		qspi_status = QSPI_TRANSFER_ERROR;
	}
	if (QUADSPI->SR & QUADSPI_SR_TCF) {	// Transfer Complete
		QUADSPI->FCR |= QUADSPI_FCR_CTCF;

		qspi_status = QSPI_SUCCESSFUL;
		QUADSPI->CCR &= ~(
			  QUADSPI_CCR_FMODE_Msk
			| QUADSPI_CCR_DMODE_Msk
			| QUADSPI_CCR_ADMODE_Msk
			| QUADSPI_CCR_IMODE
			| QUADSPI_CCR_INSTRUCTION_Msk);
		if (qspi_dma_use) {
			qspi_dma_use = false;
			DMA2_Channel7->CCR &= ~DMA_CCR_EN;
		}
	}
	if (QUADSPI->SR & QUADSPI_SR_TOF) {	// Timeout
		QUADSPI->FCR |= QUADSPI_FCR_CTOF;

		// isn't used as Memory-Mapped mode isn't used
	}
}
