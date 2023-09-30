#include "timers.h"
#include "../../globals.h"

void pwm_timer_gpio() {
	RCC->AHB2ENR  |= RCC_AHB2ENR_GPIODEN;
	GPIOD->MODER  &= ~GPIO_MODER_MODE12_Msk;
	GPIOD->MODER  |= (2 << GPIO_MODER_MODE12_Pos);
	GPIOD->AFR[1] |= (2 << GPIO_AFRH_AFSEL12_Pos);
}

bool init_pwm_timer(uint32_t frequency) {
	pwm_timer_gpio();
	RCC->APB1ENR1 |= RCC_APB1ENR1_PWM_TIMEN;

	PWM_TIM->PSC = 79;		// timer clock is at 1 microsecond
	PWM_TIM->ARR = frequency;

	PWM_TIMER->EGR |= TIM_EGR_UG;
	PWM_TIMER->CCMR1 = (6 << TIM_CCMR1_OC1M_Pos);
	PWM_TIMER->CCER |= TIM_CCER_CC1E;
}

void set_duty_cycle(uint32_t cycle) {
	// TODO: Make Percetage?? Needs to store frequency for that

	PWM_TIMER->CCR1 = cycle;
}
