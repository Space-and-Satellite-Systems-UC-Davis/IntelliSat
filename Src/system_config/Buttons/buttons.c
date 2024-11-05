/*
 * buttons.c
 *
 * 	- January    6, 2024
 * 		Author	: Darsh
 * 		Log		: get_buttonStatus functions for button management
 *
 *  - September 23, 2023 (Creation)
 *  	Author	: Darsh
 *  	Log		: Moved all button related code in one place
 */


#include "buttons.h"

// Global variables
bool button0;
bool button1;

/*************************** Buttons Initializers ****************************/

/**
 * Initializes the EXTI controller for pins PB10 and PB11
 *
 * @param None
 *
 * @returns None
 */
void button_interruptConfig() {

#if OP_REV == 1 || OP_REV == 2

	/*
	 * For Buttons on PB10 and PB11
	 * 		- Set the EXTIx controller to listen to PBx (through SYSCFG)
	 * 		- Enable the interrupt masks for both Pin x
	 * 		- Set to interrupt at rising clock edge
	 * 			- And specifically not at falling clock edge.. messy if both happen
	 */

	// PB10
	SYSCFG->EXTICR[2] &= ~SYSCFG_EXTICR3_EXTI10;
	SYSCFG->EXTICR[2] |= SYSCFG_EXTICR3_EXTI10_PB;
	EXTI->IMR1 |= EXTI_IMR1_IM10;
	EXTI->RTSR1 |= EXTI_RTSR1_RT10;
	EXTI->FTSR1 &= ~EXTI_FTSR1_FT10;

	// PB11
	SYSCFG->EXTICR[2] &= ~SYSCFG_EXTICR3_EXTI11;
	SYSCFG->EXTICR[2] |= SYSCFG_EXTICR3_EXTI11_PB;
	EXTI->IMR1 |= EXTI_IMR1_IM11;
	EXTI->RTSR1 |= EXTI_RTSR1_RT11;
	EXTI->FTSR1 &= ~EXTI_FTSR1_FT11;

#endif

}

void buttons_init() {

#if OP_REV == 1 || OP_REV == 2

	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
	while (GPIOB->OTYPER == 0xFFFFFFFF);

	// configure Input mode for BTN0 and BTN1
	GPIOB->MODER = 0x00000000;	// TODO
	// configure Pull-Up mode for BTN0 and BTN1
	GPIOB->PUPDR |=
		  GPIO_PUPDR_PUPD10_0	// BTN0
		| GPIO_PUPDR_PUPD11_0;	// BTN1

	button_interruptConfig();

	// Set a high priority for pin 10-15 interrupts, and enable the interrupts
	NVIC_SetPriority(EXTI15_10_IRQn, 1);
	NVIC_EnableIRQ(EXTI15_10_IRQn);

#endif

}

bool get_buttonStatus_0() {
	if (button0 == true) {
		button0 = false;
		return true;
	} else {
		return false;
	}
}

bool get_buttonStatus_1() {
	if (button1 == true) {
		button1 = false;
		return true;
	} else {
		return false;
	}
}

/************************* Button Interrupt Handlers *************************/

void Button0_Handler(){
	button0 = true;
}

void Button1_Handler(){
	button1 = true;
}
