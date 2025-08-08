/*
 * sleep.c
 *
 *  Created on: Jul 16, 2025
 */
#include "sleep.h"

#include <print_scan.h>

void PWR_enterLPRunMode() {
	//Optionally, we could power down flash here

	RCC->CFGR &= ~(RCC_CFGR_SW);
	RCC->CFGR |= (RCC_CFGR_SW_MSI);
//	RCC->CR |= RCC_CR_MSIRANGE_0;
//	RCC->CR |= RCC_CR_MSIRGSEL;

//	RCC->CFGR &= ~(RCC_CFGR_HPRE_Msk);
//	RCC->CFGR |= RCC_CFGR_HPRE_DIV64; //Divide SYSCLK by 64. 80MHz -> 1.25MHz

//	RCC->CFGR &= ~(RCC_CFGR_PPRE1_Msk);
//	RCC->CFGR |= RCC_CFGR_PPRE1_DIV16;

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

//	__disable_irq();
	// Disabling all external interrupts doesn't apply to SysTick
//	NVIC_DisableIRQ(EXTI15_10_IRQn);
	SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;

//	//Enable timer which will wake us up
//    RCC->APB1ENR1 |= RCC_APB1ENR1_LPTIM1EN;
//    NVIC_EnableIRQ(LPTIM1_IRQn);
//
//    RCC->APB1RSTR1 |=  (RCC_APB1RSTR1_LPTIM1RST);
//    RCC->APB1RSTR1 &= ~(RCC_APB1RSTR1_LPTIM1RST);
//    RCC->CCIPR |= (0b11) << RCC_CCIPR_LPTIM1SEL_Pos;
//
//    uint32_t core_clock_hz = 1250000;
//    LPTIM1->CFGR |= LPTIM_CFGR_COUNTMODE;
//    LPTIM1->CFGR |= (0b100) << LPTIM_CFGR_PRESC_Pos;
//
//    LPTIM1->ARR = 10000;
//
//    LPTIM1->IER |= LPTIM_IER_ARROKIE;
//
//    LPTIM1->CR |= LPTIM_CR_ENABLE;

	rtc_wakeUp(1);

    __DSB();
	__WFI();
}

void PWR_exitLPSleepMode() {

}
