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

#include "../../globals.h"
#include "timers.h"
#include "../LED/led.h"


// Global (external) variables and functions
extern int core_MHz;	// from core_config.h

uint64_t systick_time = 0;

uint64_t getSysTime() {
	return systick_time;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
/*                                 SYSTICK                                   */
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

/**
 * Starts the Systick, responsible for the global timer & the the blinking
 * of the status LEDs.
 *
 * @param   None
 * @returns None
 */
void systick_init() {
	// configure for 1 ms period
	SysTick->LOAD = (core_MHz / 8) * 1000;
	// use AHB/8 as input clock, and enable counter interrupt
	SysTick->CTRL = 0x3;
	NVIC_EnableIRQ(SysTick_IRQn);
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
	// scheduler();
}

