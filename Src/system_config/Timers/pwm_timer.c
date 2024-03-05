/*
 * pwm_timers.c
 *
 *  - Nov 8-9, 2023
 *      Author       : nithinsenthil
 *      Log          : Updated pwm timer GPIO config for OP Rev2
 *
 *  - Unknown (Creation)
 *      Author       : Unknown
 *      Contributors : Unknown
 *      Log          : ESC controller functions
 */

#include "timers.h"
#include <globals.h>

// Global (external) variables and functions
extern int core_MHz;	// from core_config.h

void pwm_timer_gpio() {

#if OP_REV == 2

	/* OP R2 GPIO pinout
	 * 		TIM CH1		GPIO A15	AF - 1
	 */

	// Clock setup
	RCC->AHB2ENR  |= RCC_AHB2ENR_GPIOAEN;

	// Reset pin state
	GPIOA->MODER  &= ~GPIO_MODER_MODE15_Msk;
	GPIOA->AFR[1] &= ~GPIO_AFRH_AFSEL15_Msk;

	// Set pin mode
	GPIOA->MODER  |= (2U << GPIO_MODER_MODE15_Pos);

	// Set AF
	GPIOA->AFR[1] |= (1U << GPIO_AFRH_AFSEL15_Pos);

#elif OP_REV == 1

	/* OP R1 GPIO pinout
	 * 		TIM CH1		GPIO D12	AF - 2
	 */

	RCC->AHB2ENR  |= RCC_AHB2ENR_GPIODEN;
	GPIOD->MODER  &= ~GPIO_MODER_MODE12_Msk;
	GPIOD->MODER  |= (2U << GPIO_MODER_MODE12_Pos);
	GPIOD->AFR[1] |= (2U << GPIO_AFRH_AFSEL12_Pos);

# endif

}


bool pwm_initTimer(uint32_t period) {
	pwm_timer_gpio();
	PWMTimer_ClockEnable();

	// (f_clk / f_goal) - 1  ==> ( 2*(core_MHz/16) / 1 MHz) - 1
	// (2*(core_MHz/16)) due to RCC configurations in core_config.c
 	PWMTimer->PSC = 2*(core_MHz/16) - 1;		// Frequency in 1us;
	PWMTimer->ARR = period;

	PWMTimer->EGR |= TIM_EGR_UG;
	PWMTimer->CCMR1 = (6 << TIM_CCMR1_OC1M_Pos);
	PWMTimer->CCER |= TIM_CCER_CC1E;

	return true;
}

void pwm_setDutyCycle(uint8_t percentage) {
	uint32_t period = PWMTimer->ARR;
	PWMTimer->CCR1 = (int)(percentage * (period/100));
}
