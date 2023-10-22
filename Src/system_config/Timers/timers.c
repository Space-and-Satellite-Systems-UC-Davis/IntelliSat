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
#include "../../tools/print_scan.h"

// initializing Global (external) variables
int systick_time = 0;
int ag_counter = 0;
int heartbeat_counter = 0;

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
/*                               HEARTBEAT - SYSTICK                         */
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

/**
 * Starts the Systick, responsible for the global timer & the the blinking
 * of the status LEDs.
 *
 * @param   None
 * @returns None
 */
void heartbeat_init() {
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

	// toggle LEDs
	if (!(systick_time % 1000)) {
		heartbeat_counter = 100;
	}
	if (!(systick_time % 250)) {
		ag_counter = 10;
	}
	if (heartbeat_counter) {
		op_led_hb(1);
		heartbeat_counter--;
	} else {
		op_led_hb(0);
	}
	if (ag_counter) {
		op_led_ag(1);
		ag_counter--;
	} else {
		op_led_ag(0);
	}
}


//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
/*                                RAW INTERRUPTS                            */
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

