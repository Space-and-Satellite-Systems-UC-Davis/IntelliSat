/*
 * core_config.h
 *
 *  - September 23, 2023
 *  	Author	: Darsh
 *  	Log		: file name change
 *
 *  - April 29, 2023
 *      Author       : Tim S.
 *      Contributors : nithinsenthil , Raphael
 *      Log          : All necessary clock registers configured
 */

#ifndef REALOP1_CORE_CONFIG_H_
#define REALOP1_CORE_CONFIG_H_

#include "stm32l476xx.h"
#include <globals.h>

enum rcc_lpuart1_clk_sources {
    RCC_CCIPR_LPUART1SEL_PCLK,
    RCC_CCIPR_LPUART1SEL_SYSCLK,
    RCC_CCIPR_LPUART1SEL_HSI16,
    RCC_CCIPR_LPUART1SEL_LSE
};

enum rcc_uart_and_usart_clk_sources {
    RCC_CCIPR_UARTSEL_PCLK,
    RCC_CCIPR_UARTSEL_SYSCLK,
    RCC_CCIPR_UARTSEL_HSI16,
    RCC_CCIPR_UARTSEL_LSE
};

enum rcc_cfgr_ppre_prescalers {
    RCC_CFGR_PPRE_APB_HCLK_NO_DIV = 0,
    RCC_CFGR_PPRE_APB_HCLK_DIV_2 = 4,
    RCC_CFGR_PPRE_APB_HCLK_DIV_4 = 5,
    RCC_CFGR_PPRE_APB_HCLK_DIV_8 = 6,
    RCC_CFGR_PPRE_APB_HCLK_DIV_16 = 7
};

/**
 * Modify clock frequency and account for any timers that need to be notified.
 *
 * @param   None
 * @returns None
 */
void changeCore_Hz(int Hz);

/**
 * Initializes the clocks of the micro-controller.
 *
 * This function sets up the various clock sources and
 * frequencies for the micro-controller.
 *
 * @param   None
 * @returns None
 */
void init_coreClocks();

/**
 * Enables writing access to registers powered by the Backup Domain
 * Key registers include RCC's BDRC, and several key RTC registers
 *
 * @param   None
 * @returns None
 */
void backup_domain_controlEnable();

/**
 * Disables writing access to registers powered by the Backup Domain
 * Key registers include RCC's BDRC, and several key RTC registers
 *
 * @param   None
 * @returns None
 */
void backup_domain_controlDisable();

#endif /* REALOP1_CORE_CONFIG_H_ */
