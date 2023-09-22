/*
 *
 */

#include "qspi.h"

short int qspi_status;
bool qspi_in_use = false;
bool timeout = false;

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

bool qspi_set_command(uint8_t fmode, uint8_t imode, uint8_t admode, uint8_t abmode, uint8_t dcyc, uint8_t dmode) {
	if (qspi_in_use) {
		return false;
	}
	qspi_disable();
	QUADSPI->CCR &=
		  ~(QUADSPI_CCR_FMODE)
		& ~(QUADSPI_CCR_IMODE)
		& ~(QUADSPI_CCR_ADMODE)
		& ~(QUADSPI_CCR_ABMODE)
		& ~(QUADSPI_CCR_DCYC)
		& ~(QUADSPI_CCR_DMODE);
	QUADSPI->CCR |=
		  (fmode << QUADSPI_CCR_FMODE_Pos)
		| (imode << QUADSPI_CCR_IMODE_Pos)
		| (admode << QUADSPI_CCR_ADMODE_Pos)
		| (abmode << QUADSPI_CCR_ABMODE_Pos)
		| (dcyc << QUADSPI_CCR_DCYC_Pos)
		| (dmode << QUADSPI_CCR_DMODE_Pos);
	qspi_in_use = true;
	return true;
}

bool qspi_send_command(
		uint8_t instruction,
		uint32_t address,
		uint32_t data_length,
		uint8_t *data,
		bool r_or_w,
		uint32_t timeout_period
) {
	if (!qspi_in_use) {
		return false;
	}
	QUADSPI->CCR &= ~QUADSPI_CCR_INSTRUCTION_Msk;
	if (timeout_period > 0) {
		// TODO: Set Time Out + Interrupt
	}
	qspi_enable();

	QUADSPI->DLR =  data_length - 1;
	QUADSPI->CCR |= (instruction << QUADSPI_CCR_INSTRUCTION_Pos);
	QUADSPI->AR  =   address;

	for (uint64_t i = 0; i < data_length; i++) {
		while (!(QUADSPI->SR & QUADSPI_SR_FTF)) {
			if ((r_or_w == true) && (QUADSPI->SR & QUADSPI_SR_TCF)) {
				break;
			}
			if (qspi_status == QSPI_TIMEDOUT) {
				// TODO: abort
				goto qspi_send_command_complete;
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

	qspi_in_use = false;
	return true;
}

// Status-Polling Mode for 4 wire QSPI
bool qspi_status_poll(
		uint8_t instruction,
		uint8_t mask,
		uint8_t match,
		uint32_t timeout_period
) {
	if (qspi_in_use) {
		return false;
	}
	qspi_in_use = true;

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

	if (timeout_period > 0) {
		// TODO: Set Time Out + Interrupt
	}

	qspi_enable();
	QUADSPI->CCR |=
		  (2U << QUADSPI_CCR_FMODE_Pos)
		| (3U << QUADSPI_CCR_DMODE_Pos)
		| (3U << QUADSPI_CCR_ADMODE_Pos)
		| (1U << QUADSPI_CCR_IMODE_Pos)
		| (instruction << QUADSPI_CCR_INSTRUCTION_Pos);

	// while (QUADSPI->SR & QUADSPI_SR_BUSY);

	/*
	 * Once the Instruction is written, QSPI peripheral will keep polling
	 * until the match is found. Then the interrupt is responsible for
	 * clearing the flag, stopping the status polling mode, and stopping
	 * the peripheral
	 * Until then, the qspi_in_use variable will indicate
	 * that the peripheral can't be used.
	 */

	return true;
}







#define qspi_stop_status_polling() \
		QUADSPI->CCR  &= ~(QUADSPI_CCR_FMODE | QUADSPI_CCR_DMODE); \
		QUADSPI->CR   &= ~QUADSPI_CR_APMS

void QUADSPI_IRQHandler() {
	if (QUADSPI->SR & QUADSPI_SR_SMF) {	// Status Polling complete
		QUADSPI->FCR |= QUADSPI_FCR_CSMF;
		QUADSPI->CR  &= ~QUADSPI_CR_SMIE;

		qspi_stop_status_polling();
		qspi_disable();
		qspi_in_use = false;
	}
	if (QUADSPI->SR & QUADSPI_SR_TEF) {	// Transfer Error
		QUADSPI->FCR |= QUADSPI_FCR_CTEF;

		// TODO
	}
	if (QUADSPI->SR & QUADSPI_SR_TCF) {	// Transfer Complete
		QUADSPI->FCR |= QUADSPI_FCR_CTCF;
	}
	if (QUADSPI->SR & QUADSPI_SR_TOF) {	// Timeout
		QUADSPI->FCR |= QUADSPI_FCR_CTOF;
		QUADSPI->CR  &= ~QUADSPI_CR_TEIE;


		if (QUADSPI->CCR & QUADSPI_CCR_FMODE == 2U) {
			qspi_stop_status_polling();
		} else {
			timeout = true;
		}
		qspi_disable();
		qspi_in_use = false;
	}
}
