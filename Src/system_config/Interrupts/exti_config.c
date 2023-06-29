/*
 * exti_config.c
 *
 *  - May 13-16, 2023
 * 		Author       : Raphael, Darsh, Parteek
 *      Contributors : nithilsenthil, Huey, Raymond, Kevin
 *      Log          : wrote the EXTI config and button config
 */

#include "exti_config.h"

/*
 * Initializes the EXTI controller for pins PB10 and PB11
 *
 * @param None
 *
 * @returns None
 */
void button_interrupt_config() {
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
}

/*
 * Initializes the EXTI controllers to intercept and pass on any GPIO pin interrupts
 *
 * @param None
 *
 * @returns None
 */
void EXTI_init(){
	// configure the EXTI registers for the button (PB10 & PB11) interrupts
	button_interrupt_config();
}
