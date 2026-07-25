/*
 * buttons.h
 *
 *  - January    6, 2024
 *      Author  : Darsh
 *      Log     : get_buttonStatus functions for button management
 *
 *  - September 23, 2023 (Creation)
 *      Author  : Darsh
 *      Log     : Moved all button related code in one place
 */

#include <globals.h>
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

/**
 * Returns the status of whether Button 0 has been pressed or not.
 *
 * @param   None
 * @returns boolean for whether the button was pressed once
 */
bool get_buttonStatus_0();

/**
 * Returns the status of whether Button 1 has been pressed or not.
 *
 * @param   None
 * @returns boolean for whether the button was pressed once
 */
bool get_buttonStatus_1();

/**
 * Returns the status of whether Button SW1 has been pressed or not.
 *
 * @param   None
 * @returns boolean for whether the button was pressed once
 */
bool get_buttonStatus_SW1();

/************************* Button Interrupt Handlers *************************/

/**
 * Interrupt Handler for Button 0
 *
 * @param None
 * @returns None
 */
void Button0_Handler();

/**
 * Interrupt Handler for Button 1
 *
 * @param None
 * @returns None
 */
void Button1_Handler();

/**
 * Interrupt Handler for Button SW1
 *
 * @param None
 * @returns None
 */
void ButtonSW1_Handler();



