/*
 * sleep.c
 *
 *  Created on: Jul 16, 2025
 */
#include "sleep.h"

void PWR_enterLPRunMode() {
	//Optionally, we could power down flash here

	RCC->CFGR &= ~(RCC_CFGR_HPRE_Msk);
	RCC->CFGR |= RCC_CFGR_HPRE_DIV64; //Divide SYSCLK by 64. 80MHz -> 1.25MHz

	PWR->CR1 |= PWR_CR1_LPR;
}
bool is_REGLPF_not_clear() { return (PWR->SR2 & PWR_SR2_REGLPF) != 0; }
void PWR_exitLPRunMode() {
	PWR->CR1 &= ~(PWR_CR1_LPR);

	wait_with_timeout(is_REGLPF_not_clear, DEFAULT_TIMEOUT_MS);

	RCC->CFGR &= RCC_CFGR_HPRE_Msk; // Reset clock
}

void PWR_enterLPSleepMode() {
	PWR_enterLPRunMode();

	// Turn off SysTick since it wakes us up
	for (int i = -14; i < 82; i++) {
//		__disable_irq();
		NVIC_DisableIRQ(i);
		NVIC_ClearPendingIRQ(i);
//		NVIC_EnableIRQ(i);
	}
//	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;

	__WFI();
}
