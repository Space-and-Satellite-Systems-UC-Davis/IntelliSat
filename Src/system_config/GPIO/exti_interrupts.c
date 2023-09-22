/*
 * interrupt_handlers.c
 *
 * 	- May 14-16, 2023
 * 		Author       : Raphael, Darsh, Parteek
 *      Contributors : nithilsenthil, Huey, Raymond, Kevin
 *      Log          : wrote the Button Handlers
 *
 *  - Apr 29, 2023 (Creation)
 *      Author       : Tim S.
 *      Contributors : nithinsenthil , Raphael
 *      Log          : wrote the Systick_Handler
 */

#include "../../peripherals/LED/led.h"
#include "../clock_nvic_config.h"
#include "../../tools/print_scan.h"


// initializing Global (external) variables
int systick_time = 0;	// systick interrupt counter

// prototypes for functions defined later
void Button0_Handler();
void Button1_Handler();

/**
 * Interrupt handler for the SysTick timer.
 * Increments the systick_time variable and updates the status of the heartbeat and activity LEDs.
 *
 * @param None
 *
 * @returns None
 */
void SysTick_Handler() {
	systick_time++;

	// heartbeat led
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

/*
 * Interrupt Handler for GPIO Pins 10 - 15 (all ports)
 * Figures out which pin caused the interrupt, then calls it's respective handler
 *
 * @param None
 *
 * @returns None
 */
void EXTI15_10_IRQHandler(){
	/*
	 * General Procedure
	 * 		- Check which pins caused the interrupt by checking the Pending Request Register
	 * 		- For the request that is pending, clear the interrupt flag by writing a
	 * 		  1 to the same Pending Request Register
	 * 		- Call the Interrupt Handler for whatever is connected to that pin (or the pin itself)
	 */

	if (EXTI->PR1 & EXTI_PR1_PIF10) {		// Button 0
		EXTI->PR1 |= EXTI_PR1_PIF10;
		Button0_Handler();
	}
	else if (EXTI->PR1 & EXTI_PR1_PIF11) {	// Button 1
		EXTI->PR1 |= EXTI_PR1_PIF11;
		Button1_Handler();
	}
}

extern button;
/**
 * Interrupt Handler for Button 0
 * Prints a message to the console
 *
 * @param None
 *
 * @returns None
 */
void Button0_Handler(){
	button = true;
}

/**
 * Interrupt Handler for Button 1
 * Prints a message to the console
 *
 * @param None
 *
 * @returns None
 */
void Button1_Handler(){
	printMsg("Button 1 Pressed!\n");
}


