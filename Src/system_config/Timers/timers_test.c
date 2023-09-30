#include "timers.h"

void branch_main() {
	/*
	 * TIM4->PRSC = Whatever...
	 * TIM4->ARR = Frequency (based on PRSC)
	 * TIM4->CCR = %duty_cycle * ARR
	 *
	 *  - Start the Timer, set it's PRESCALER & ARR (these control the frequency of the PWM)
	 *  - Set UG so the timer updates itself to the new values
	 *
	 *  - Set Duty Cycle using CCR1
	 *  - Set PWM Mode (OC1M) using CCMR1
	 *
	 *  - Set Polarity in CCER
	 *  - Start PWM mode (CC1E) in CCER
	 */
	RCC->AHB2ENR  |= RCC_AHB2ENR_GPIODEN;
	GPIOD->MODER  &= ~GPIO_MODER_MODE12_Msk;
	GPIOD->MODER  |= (2 << GPIO_MODER_MODE12_Pos);
	GPIOD->AFR[1] |= (2 << GPIO_AFRH_AFSEL12_Pos);

	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM4EN;

	// TIM4->CR1 |= TIM_CR1_URS;

	TIM4->PSC = 79;		// timer clock is at 1 microsecond
	TIM4->ARR = 500;	// frequency = 0.5 ms

	TIM4->EGR |= TIM_EGR_UG;

	TIM4->CCR1 = 400;	// 80% duty cycle
	TIM4->CCMR1 = (6 << TIM_CCMR1_OC1M_Pos);
//	TIM4->CCMR1 |= TIM_CCMR1_OC1PE;
//
//	TIM4->CR1 |= TIM_CR1_ARPE;

	TIM4->CCER |= TIM_CCER_CC1E;
	TIM4->CR1 |= TIM_CR1_CEN;

	while (1) {

	}
}
