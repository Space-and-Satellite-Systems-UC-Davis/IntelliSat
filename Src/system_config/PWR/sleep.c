/*
 * sleep.c
 *
 *  Created on: Jul 16, 2025
 */
#include "sleep.h"

#include <print_scan.h>

void PWR_enterLPRunMode() {
	//Optionally, we could power down flash here

	RCC->CFGR &= ~(RCC_CFGR_HPRE);
	// Divide SYSCLK by 64. 80MHz -> 1.25MHz
	// Assuming PLL
	RCC->CFGR |= RCC_CFGR_HPRE_DIV64;

	PWR->CR1 |= PWR_CR1_LPR;
}
bool is_REGLPF_not_clear() { return (PWR->SR2 & PWR_SR2_REGLPF) != 0; }
void PWR_exitLPRunMode() {
	PWR->CR1 &= ~(PWR_CR1_LPR);

	wait_with_timeout(is_REGLPF_not_clear, DEFAULT_TIMEOUT_MS);

	RCC->CFGR &= ~(RCC_CFGR_HPRE); // Reset clock divisor
}

void PWR_enterLPSleepMode(uint16_t seconds) {
	PWR_enterLPRunMode();

	//Could disable all interrupts here

	// Disabling all external interrupts doesn't apply to SysTick
	SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;

	rtc_wakeUp(seconds);

	SCB->SCR &= ~(SCB_SCR_SLEEPDEEP_Msk);
    __DSB();
	__WFI();
}

void PWR_exitLPSleepMode() {
	PWR_exitLPRunMode();
}
