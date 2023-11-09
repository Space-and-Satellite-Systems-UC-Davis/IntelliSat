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
#include "../../globals.h"

# ifdef OP_REV2

void pwm_timer_gpio() {
	// GPIO
	/* OP R2 GPIO pinout
	 * 		ESC TMR2 CH1	A15		(Alternate Function, AF1)
	 */

	// TODO : Allow this to be variable

	// Clock setup
	RCC->AHB2ENR  |= RCC_AHB2ENR_GPIOAEN;

	// Reset pin state
	GPIOA->MODER  &= ~GPIO_MODER_MODE15_Msk;
	GPIO->AFR[1] &= ~GPIO_AFRL_AFSEL15_Msk;

	// Set pin mode
	GPIOA->MODER  |= (2U << GPIO_MODER_MODE15_Pos);

	// Set AF
	GPIOA->AFR[1] |= (1U << GPIO_AFRH_AFSEL15_Pos);
}

# else

void pwm_timer_gpio() {
	// TODO : Allow this to be variable
	RCC->AHB2ENR  |= RCC_AHB2ENR_GPIODEN;
	GPIOD->MODER  &= ~GPIO_MODER_MODE12_Msk;
	GPIOD->MODER  |= (2 << GPIO_MODER_MODE12_Pos);
	GPIOD->AFR[1] |= (2 << GPIO_AFRH_AFSEL12_Pos);
}

# endif

bool init_pwm_timer(uint32_t period) {
	pwm_timer_gpio();
	EscControlTimer_ClockEnable();

	// (f_clk / f_goal) - 1  ==> ( 2*(core_MHz/16) / 1 MHz) - 1
	// (2*(core_MHz/16)) due to RCC configurations in core_config.c
 	EscControlTimer->PSC = 2*(core_MHz/16) - 1;		// Frequency in 1us;
	EscControlTimer->ARR = period;

	EscControlTimer->EGR |= TIM_EGR_UG;
	EscControlTimer->CCMR1 = (6 << TIM_CCMR1_OC1M_Pos);
	EscControlTimer->CCER |= TIM_CCER_CC1E;

	return true;
}

void set_duty_cycle(uint8_t percentage) {
	uint32_t period = EscControlTimer->ARR;
	EscControlTimer->CCR1 = (int)(percentage * (period/100));
}
