/*
 * sleep.h
 *
 *  Created on: Jul 16, 2025
 */

#ifndef SYSTEM_CONFIG_PWR_SLEEP_H_
#define SYSTEM_CONFIG_PWR_SLEEP_H_

#include "stm32l476xx.h"
#include <globals.h>
#include "RTC/rtc.h"
#include "Timers/timers.h"

typedef enum OperatingMode {
	RUN, LPRUN, LPSLEEP
} OperatingMode;

/**
 * @brief Enter Low Power Run mode
 *
 * @param None
 *
 * @returns None
 */
void PWR_enterLPRunMode();

/**
 * @brief Exit low power mode
 * @param None
 * 
 * @returns None
 */
void PWR_exitLPRunMode();

/**
 * Enter or exit Low Power Sleep Mode
 *
 * @param seconds    How long to go to sleep for
 *
 * @returns false if RTC isn't using LSE or LSI
 */
bool PWR_enterLPSleepMode(uint16_t seconds);

/**
 * Exit Low Power Sleep Mode
 *
 * @returns false if RTC isn't using LSE or LSI
 */
void PWR_exitLPSleepMode();

/**
 * By default, program will exit LPSleep upon interrupt.
 * If you don't want that, call this function at the end of
 * the interrupt.
 * See RTC_WKUP_IRQHandler as example
 *
 * @param None
 *
 * @returns None
 */
void PWR_maintainLPSleep();

#endif /* SYSTEM_CONFIG_PWR_SLEEP_H_ */
