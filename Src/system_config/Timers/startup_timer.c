/*
 * startup_timer.c
 *
 *	9th March, 2024
 * 		Author : Darsh
 * 		Log    : Creation, timer_initStartupTimer(), timer_waitStartupTime(), Interrupt Handler
 */

#include "timers.h"

#define timer_startupTimerOn()	StartupTimer->CR1 |=  TIM_CR1_CEN;
#define timer_startupTimerOff()	StartupTimer->CR1 &= ~TIM_CR1_CEN;


#define MAX_STARTUP_COUNT (30 * 60)		// 30 minutes * 60 seconds per minute

int startup_count = 0;


void timer_initStartupTimer() {
	// R = F / [(ARR+1)(PSC+1)(RCR+1)]
	// https://docs.google.com/document/d/1TprKrSQ9GlH2M5u8z4JouCm3d4kw6UXFi6ckE5Hslgk/edit?usp=sharing
	// 		F   =  5 * 10^6
	// 		R   =  1			<-- 1 second
	//		RCR =  0

	// Prescalar
	uint32_t PSC = 1065;		// Random Number
	// Auto Reload Counter
	uint32_t ARR = 4695;		// (F/(R(PSC + 1)) - 1)

	StartupTimer_ClockEnable();
	timer_startupTimerOff();

	StartupTimer->PSC = PSC;
	StartupTimer->ARR = ARR;

	StartupTimer->CR1 &= ~TIM_CR1_DIR;
	StartupTimer->CR1 |=  TIM_CR1_URS;

	// Enable timer interrupt
	StartupTimer->DIER |= TIM_DIER_UIE;
	// Enable TIM6 interrupt
	NVIC_EnableIRQ(TIM5_IRQn);
}


/**
 * Configures the Startup Timer registers to generate
 * an interrupt in 30 minutes
 *
 * @returns Boolean to indicate if the initialization was successful
 */
void timer_waitStartupTime() {
	timer_initStartupTimer();

	timer_startupTimerOn();
	while (startup_count < MAX_STARTUP_COUNT);
	timer_startupTimerOff();
}


void TIM5_IRQHandler() {
	if(TIM5->SR & TIM_SR_UIF) {
		TIM5->SR &= ~TIM_SR_UIF;
		startup_count++;
	}
}
