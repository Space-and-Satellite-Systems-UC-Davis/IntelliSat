/*
 * buttons.h
 *
 *  - September 23, 2023 (Creation)
 *  	Author	: Darsh
 *  	Log		: Moved all button related code in one place
 */

#include "stm32l476xx.h"

/*************************** Buttons Initializers ****************************/

/*
 * Initializes the GPIO for the buttons 0 & 1.
 * Also enables interrupts on both
 *
 * @param   None
 * @returns None
 */
void buttons_init();

/************************* Button Interrupt Handlers *************************/

void Button0_Handler();
void Button1_Handler();
