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

void PWR_enterLPRunMode();
void PWR_exitLPRunMode();

void PWR_enterLPSleepMode();

#endif /* SYSTEM_CONFIG_PWR_SLEEP_H_ */
