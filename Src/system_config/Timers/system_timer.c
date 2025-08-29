/*
 * timers.c
 *
 *	- October 11, 2023
 *		Author	: Darsh
 *		Log		: ...
 *
 *  - September 23, 2023
 *  	Author	: Darsh
 *  	Log		: Initial Setup
 */

#include <globals.h>
#include "timers.h"
#include <LED/led.h>


// Global (external) variables and functions
extern int core_Hz;	// from core_config.h

uint64_t systick_time = 0;

void delay_ms(uint64_t ms) {
	uint64_t start_time = systick_time;
	while (systick_time - start_time < ms);
}

//Returns elapsed ms
uint64_t getSysTime() {
	return systick_time;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
/*                                 SYSTICK                                   */
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

bool _run_scheduler;

void systick_adjust_reload() {
	// configure for 1 ms period
	// frequency/1000 = cycles/ms
	// frequenchy = AHB/8 = SYSCLK/8 = coreHz/8
	SysTick->LOAD = core_Hz/8/1000;
}

void systick_init(bool run_scheduler) {
	// configure for 1 ms period
	systick_adjust_reload();

	// use AHB/8 as input clock, and enable counter interrupt
	SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
	SysTick->CTRL &= ~SysTick_CTRL_CLKSOURCE_Msk;	// CLKSOURCE of 0 uses AHB/8
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;

	// SysTick isn't controlled by NVIC in this way
	// I don't know why this is here
	NVIC_EnableIRQ(SysTick_IRQn);

	_run_scheduler = run_scheduler;
}


/**
 * Interrupt handler for the SysTick timer.
 * Increments the systick_time variable and
 * updates the status of the heartbeat and activity LEDs.
 *
 * @param None
 * @returns None
 */
void SysTick_Handler() {
	systick_time++;
	blinky();
	if (_run_scheduler == true) {
		// scheduler();
	}
}
