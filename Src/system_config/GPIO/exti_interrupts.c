/*
 * exti_interrupts.c
 *
 *  - September 23, 2023
 *  	Author 	: Darsh
 *  	Log		: changed to just contain EXTI interrupt handlers
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

#include <Buttons/buttons.h>

/*
 * Interrupt Handler for GPIO Pins 10 - 15 (all ports)
 * Figures out which pin caused the interrupt, then calls
 * it's respective handler
 *
 * @param None
 *
 * @returns None
 */
void EXTI15_10_IRQHandler(){
	/*
	 * General Procedure
	 * 		- Check which pins caused the interrupt
	 * 		  by checking the Pending Request Register
	 * 		- For the request that is pending, clear the interrupt flag
	 * 		  by writing a 1 to the same Pending Request Register
	 * 		- Call the Interrupt Handler for whatever
	 * 		  is connected to that pin (or the pin itself)
	 */

	if (EXTI->PR1 & EXTI_PR1_PIF10) {		// Button 0
		EXTI->PR1 |= EXTI_PR1_PIF10;
#if OP_REV == 1 || OP_REV == 2
		Button0_Handler();
#endif
	}
	else if (EXTI->PR1 & EXTI_PR1_PIF11) {	// Button 1
		EXTI->PR1 |= EXTI_PR1_PIF11;
#if OP_REV == 1 || OP_REV == 2
		Button1_Handler();
#endif
	}
}
