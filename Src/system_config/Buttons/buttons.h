/*
 * buttons.h
 *
 *  - September 23, 2023 (Creation)
 *  	Author	: Darsh
 *  	Log		: Moved all button related code in one place
 */

#include "../../globals.h"
#include "stm32l476xx.h"

/*************************** Buttons Initializers ****************************/

/**
 * Initializes the GPIO for the buttons 0 & 1.
 * Also enables interrupts on both
 *
 * @param   None
 * @returns None
 */
void buttons_init();

/************************* Button Interrupt Handlers *************************/

/**
 * Interrupt Handler for Button 0
 * Prints a message to the console
 *
 * @param None
 *
 * @returns None
 */
void Button0_Handler();

/**
 * Interrupt Handler for Button 1
 * Prints a message to the console
 *
 * @param None
 *
 * @returns None
 */
void Button1_Handler();
