/*
 * startup_timer.c
 *
 *	2th March, 2024
 * 		Author 		 : Basti
 * 		Contributors : Darsh
 * 		Log    		 : Creation, logger_initExpTimer(), Interrupt Handler
 */

#include "timers.h"

void (*logTimerHandler)() = NULL;

/**
 * Configures the Experiment Logger timer registers to generate
 * an interrupt every 100 ms.
 *
 * @returns Boolean to indicate if the initialization was successful
 */
bool logger_initExpTimer() {
	// R = F / [(ARR+1)(PSC+1)(RCR+1)]
	// https://docs.google.com/document/d/1TprKrSQ9GlH2M5u8z4JouCm3d4kw6UXFi6ckE5Hslgk/edit?usp=sharing
	// 		F   =  5 * 10^6
	// 		R   = 10			<-- 100 ms
	//		RCR =  0

	// Prescaler
	uint32_t PSC = 1065;		// Random Numebr
	// Auto Reload Counter
	uint32_t ARR = 468 * 2;		// (F/(R(PSC + 1)) - 1) , The doubling is due to some unknown oddity

	ExpLogTimer_ClockEnable();
	logger_expTimerOff();

	ExpLogTimer->PSC = PSC;
	ExpLogTimer->ARR = ARR;

	ExpLogTimer->CR1 &= ~TIM_CR1_DIR;
	ExpLogTimer->CR1 |= TIM_CR1_URS;

	// Enable timer interrupt
	ExpLogTimer->DIER |= TIM_DIER_UIE;
	// Enable TIM6 interrupt
	NVIC_EnableIRQ(TIM6_DAC_IRQn);

	return true;
}

void logger_registerLogFunction(void (*func)()) {
	logTimerHandler = func;
}

void TIM6_DACUNDER_IRQHandler() {
	if(TIM6->SR & TIM_SR_UIF) {
		TIM6->SR &= ~TIM_SR_UIF;
		if (logTimerHandler != NULL) {
			logTimerHandler();
		}
	}
}
