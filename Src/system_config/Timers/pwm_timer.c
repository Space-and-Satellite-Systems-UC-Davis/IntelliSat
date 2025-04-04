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
#include "GPIO/gpio.h"

// Global (external) variables and functions
extern int core_MHz;	// from core_config.h


void pwm_timer_gpio() {

#if OP_REV == 2 || OP_REV == 3

	/* OP R2 GPIO pinout
	 * 		TIM CH1		GPIO A15	AF - 1
	 *      TIM CH4     GPIO A3     AF - 1
	 */

	// Clock setup
	RCC->AHB2ENR  |= RCC_AHB2ENR_GPIOAEN;

	// Reset pin state
	GPIOA->MODER  &= ~(GPIO_MODER_MODE15_Msk | GPIO_MODER_MODE3_Msk);
	GPIOA->AFR[1] &= ~GPIO_AFRH_AFSEL15_Msk;
	GPIOA->AFR[0] &= ~GPIO_AFRL_AFSEL3_Msk;

	// Set pin mode
	GPIOA->MODER  |= (GPIO_MODER_AlternateFunction << GPIO_MODER_MODE15_Pos);
	GPIOA->MODER |= GPIO_MODER_MODE3_1;

	// Set AF
	GPIOA->AFR[1] |= (GPIO_AFRX_AF1 << GPIO_AFRH_AFSEL15_Pos);
	GPIOA->AFR[0] |= (GPIO_AFRX_AF1 << GPIO_AFRL_AFSEL3_Pos);

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
	PWMTimer->CCMR1 = (TIM_CCMR1_OC1M_PWM_MODE_1 << TIM_CCMR1_OC1M_Pos);
	PWMTimer->CCMR2 |= TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2;
	PWMTimer->CCER |= TIM_CCER_CC1E | TIM_CCER_CC4E;

	return true;
}

void pwm_setDutyCycle(PWM_Channels pwm,  uint8_t percentage) {
	uint32_t period = PWMTimer->ARR;
	if(pwm == PWM0){
		PWMTimer->CCR1 = (int)(percentage * (period/100));
	}
	else {
		PWMTimer->CCR4 = (int)(percentage * (period/100));
	}
	
}
