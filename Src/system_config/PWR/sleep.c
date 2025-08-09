/*
 * sleep.c
 *
 *  Created on: Jul 16, 2025
 */
#include "sleep.h"

#include <print_scan.h>

bool IRQ_states[82];
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

	//Store previous interrupt state and disable interrupts
	for (int i = 0; i < 82; i++) {
		(NVIC_GetEnableIRQ(i)) ? (IRQ_states[i] = true) : (IRQ_states[i] = false);
		NVIC_DisableIRQ(i);
	}

	// Disabling all external interrupts doesn't apply to SysTick
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;

	bool result = rtc_wakeUp(seconds);
	if (result == false) return false;

	SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
	mode = LPSLEEP;
	// I don't think this loop is avoidable with timeout
	// Wake up interrupt can trigger prior to getting out of LPSleep
	while(mode != RUN) {
	    __DSB();
		__WFI();
	}
	return true;
}

void PWR_exitLPSleepMode() {
	//Set the interrupts back
	for (int i = 0; i < 82; i++) {
		(IRQ_states[i]) ? (NVIC_EnableIRQ(i)) : (NVIC_DisableIRQ(i));
	}

	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;

	PWR_exitLPRunMode();
}
