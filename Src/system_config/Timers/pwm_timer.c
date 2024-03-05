#include "timers.h"
#include "../../globals.h"

void pwm_timer_gpio() {
	// TODO : Allow this to be variable
	RCC->AHB2ENR  |= RCC_AHB2ENR_GPIODEN;
	GPIOD->MODER  &= ~GPIO_MODER_MODE12_Msk;
	GPIOD->MODER  |= (2 << GPIO_MODER_MODE12_Pos);
	GPIOD->AFR[1] |= (2 << GPIO_AFRH_AFSEL12_Pos);
}

bool init_pwm_timer(uint32_t period) {
	pwm_timer_gpio();
	EscControlTimer_ClockEnable();

	// (f_clk / f_goal) - 1  ==> ( 2*(core_MHz/16) / 1 MHz) - 1
	// (2*(core_MHz/16)) due to RCC configurations in core_config.c
 	EscControlTimer->PSC = 80 - 1;		// Frequency in 1us;
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
