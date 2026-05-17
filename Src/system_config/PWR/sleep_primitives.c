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
	// Disable interrupts across the clock divider write — an ISR firing mid-transition
	// would execute at an intermediate clock speed with uncalibrated TIM7 delays.
	__disable_irq();
	RCC->CFGR &= ~(RCC_CFGR_HPRE);
	RCC->CFGR |= RCC_CFGR_HPRE_DIV64; // 80MHz -> 1.25MHz
	changeCore_Hz(1250000);
	__enable_irq();

	PWR->CR1 |= PWR_CR1_LPR;
}


static inline bool is_REGLPF_set() { return (PWR->SR2 & PWR_SR2_REGLPF) != 0; }

void PWR_exitLPRunMode(void) {
	PWR->CR1 &= ~(PWR_CR1_LPR);

	wait_with_timeout(is_REGLPF_set, DEFAULT_TIMEOUT_MS);

	__disable_irq();
	RCC->CFGR &= ~(RCC_CFGR_HPRE); // Reset clock divisor (1.25MHz -> 80MHz)
	changeCore_Hz(80000000);
	__enable_irq();
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
