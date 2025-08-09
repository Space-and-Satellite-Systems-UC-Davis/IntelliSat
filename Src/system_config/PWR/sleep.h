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

typedef enum OperatingMode {
	RUN, LPRUN, LPSLEEP
} OperatingMode;

/**
 * Enter or exit Low Power Run mode
 *
 * @param None
 *
 * @returns None
 */
void PWR_enterLPRunMode();
void PWR_exitLPRunMode();

/**
 * Enter or exit Low Power Sleep Mode
 *
 * @param seconds    How long to go to sleep for
 *
 * @returns false if RTC isn't using LSE or LSI
 */
bool PWR_enterLPSleepMode(uint16_t seconds);
void PWR_exitLPSleepMode();

#endif /* SYSTEM_CONFIG_PWR_SLEEP_H_ */
