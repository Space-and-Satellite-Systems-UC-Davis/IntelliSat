/*
 * logger_timer.c
 *
 */
#include "timers.h"
#include "../../tools/print_scan.h"

extern int core_MHz;


/**
 * Configures the Experiment Logger timer registers to generate
 * an interrupt every 100 ms.
 *
 * @returns Boolean to indicate if the initialization was successful
 */
bool logger_initExpTimer()
{
	// FORMULA
	// R = F  [(ARR+1)(PSC+1)(RCR+1)]

	// Setting ARR and PSC to get 100 ms Interrupts

	// Prescaler (using number between 0 to 2^16 bits)
	uint32_t PSC = 1065;
	// Auto Reload Counter (468 is result from chosen PSC)
	// (F/(R(PSC + 1)) - 1)
	uint32_t ARR = 468;

	// TODO
	// Enable the timer clock
	ExpLogTimer_ClockEnable();
	// Disable the timer
	logger_expTimerOff();

	// Need to set ARR and PSC values
	// Set calculated Prescaler to PSC
	ExpLogTimer->PSC = PSC;
	// Set calculated Auto Reload Counter to ARR
	ExpLogTimer->ARR = ARR;

	// Count 0 to 100ms
	// ExpLogTimer->CR1 &= ~TIM_CR1_DIR;
	ExpLogTimer->CR1 |= TIM_CR1_URS;

	// Enable timer interrupt
	ExpLogTimer->DIER |= TIM_DIER_UIE;
	// Enable TIM6 interrupt
	NVIC_EnableIRQ(TIM6_DAC_IRQn);
	// Reenable timer
	logger_expTimerOn();

	// Success
	return true;
}


void TIM6_DACUNDER_IRQHandler()
{
	// TODO
	if(TIM6->SR & TIM_SR_UIF)
	{
		// Turn off
		TIM6->SR &= ~TIM_SR_UIF;

		// Count that we printed 10 times
		printMsg("1");
	}



}
