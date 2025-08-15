/*
 * sleep.c
 *
 *  Created on: Jul 16, 2025
 */
#include "sleep.h"

#define IRQ_LENGTH 82
OperatingMode mode = RUN;

void PWR_enterLPRunMode() {
	//Optionally, we could power down flash here

	RCC->CFGR &= ~(RCC_CFGR_HPRE);
	// Divide SYSCLK by 64. 80MHz -> 1.25MHz
	// Assuming PLL
	RCC->CFGR |= RCC_CFGR_HPRE_DIV64;

	PWR->CR1 |= PWR_CR1_LPR;
	mode = LPRUN;
}
bool is_REGLPF_not_clear() { return (PWR->SR2 & PWR_SR2_REGLPF) != 0; }
void PWR_exitLPRunMode() {
	PWR->CR1 &= ~(PWR_CR1_LPR);

	wait_with_timeout(is_REGLPF_not_clear, DEFAULT_TIMEOUT_MS);

	RCC->CFGR &= ~(RCC_CFGR_HPRE); // Reset clock divisor
	mode = RUN;
}

bool PWR_enterLPSleepMode(uint16_t seconds) {
	PWR_enterLPRunMode();

	// Disable Watchdog pet sitter and tell it we are in sleep
	watchdog_IWDGSleepMode();
	NVIC_DisableIRQ(TIM3_IRQn);
	// SysTick is not handled by NVIC
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;


	bool result = rtc_wakeUp(seconds);
	if (result == false) return false;

	SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
	mode = LPSLEEP;
	// I don't think this loop is avoidable with timeout
	// Wake up interrupt can trigger prior to getting out of LPSleep
	// Loop will end unless explicitly ordered to continue
	while(mode == LPSLEEP) {
		mode = LPRUN;
		__DSB();
		__WFI();
	}
	PWR_exitLPSleepMode();
	return true;
}
void PWR_maintainLPSleep() {
	mode = LPSLEEP;
}

void PWR_exitLPSleepMode() {
	watchdog_IWDGWakeUp();
	NVIC_EnableIRQ(TIM3_IRQn);

	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;

	PWR_exitLPRunMode();
}
