/*
 * sleep.c
 *
 *  Created on: Jul 16, 2025
 */
#include "sleep_primitives.h"
#include "RTC/rtc.h"
#include "stm32l476xx.h"
#include <stdint.h>

// Global (external) variables and functions
extern int core_Hz;	// from core_config.h

void PWR_enterLPRunMode(void) {
	//Optionally, we could power down flash here

	RCC->CFGR &= ~(RCC_CFGR_HPRE);
	// Divide SYSCLK by 64. 80MHz -> 1.25MHz
	// Assuming PLL 80MHz SYSCLK
	RCC->CFGR |= RCC_CFGR_HPRE_DIV64;
	changeCore_Hz(1250000);

	PWR->CR1 |= PWR_CR1_LPR;
}


static inline bool is_REGLPF_set() { return (PWR->SR2 & PWR_SR2_REGLPF) != 0; }

void PWR_exitLPRunMode(void) {
	PWR->CR1 &= ~(PWR_CR1_LPR);

	wait_with_timeout(is_REGLPF_set, DEFAULT_TIMEOUT_MS);

	RCC->CFGR &= ~(RCC_CFGR_HPRE); // Reset clock divisor
	changeCore_Hz(80000000);
}

bool PWR_armRTC(uint16_t seconds, void (*on_cycle)()) {
	watchdog_IWDGSleepMode();
	NVIC_DisableIRQ(TIM3_IRQn);
	NVIC_DisableIRQ(TIM7_IRQn);
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
	return rtc_wakeUp(seconds, on_cycle);
}

void PWR_disarmRTC(void) {
	watchdog_IWDGWakeUp();
	NVIC_EnableIRQ(TIM3_IRQn);
	NVIC_EnableIRQ(TIM7_IRQn);
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}

void PWR_wfi(void) {
	SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
	__DSB();
	__WFI();
}

// bool PWR_enterLPSleepMode(uint16_t seconds) {
// 	PWR_enterLPRunMode();

// 	// Disable Watchdog pet sitter and tell it we are in sleep
// 	watchdog_IWDGSleepMode();
// 	NVIC_DisableIRQ(TIM3_IRQn);
// 	NVIC_DisableIRQ(TIM7_IRQn);

// 	// Set alarm to wake us up later
// 	bool result = rtc_wakeUp(seconds);
// 	if (result == false) return false;

// 	SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
// 	mode = LPSLEEP;
// 	// I don't think this loop is avoidable with timeout
// 	// Wake up interrupt can trigger prior to getting out of LPSleep
// 	// Loop will end unless explicitly ordered to continue
// 	while(mode == LPSLEEP) {
// 		mode = LPRUN;
// 		__DSB();
// 		__WFI();
// 	}
// 	PWR_exitLPSleepMode();
// 	return true;
// }
// void PWR_maintainLPSleep() {
// 	mode = LPSLEEP;
// }

// void PWR_exitLPSleepMode() {
// 	watchdog_IWDGWakeUp();
// 	NVIC_EnableIRQ(TIM3_IRQn);
// 	NVIC_EnableIRQ(TIM7_IRQn);

// 	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;

// 	PWR_exitLPRunMode();
// }
