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
extern int core_MHz;	// from core_config.h

uint64_t systick_time = 0;

void delay_ms(uint64_t ms) {
	uint64_t start_time = systick_time;
	while (systick_time - start_time < ms);
}

uint64_t getSysTime() {
	return systick_time;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
/*                                 SYSTICK                                   */
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

bool _run_scheduler;

void systick_init(bool run_scheduler) {
	// configure for 1 ms period
	SysTick->LOAD = (core_MHz / 8) * 1000;
	// use AHB/8 as input clock, and enable counter interrupt
	SysTick->CTRL = 0x3;
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
