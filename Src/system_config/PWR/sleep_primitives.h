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

/**
 * @brief Enter Low Power Run mode
 *
 * @param None
 *
 * @returns None
 */
void PWR_enterLPRunMode(void);

/**
 * @brief Exit low power mode
 * @param None
 * 
 * @returns None
 */
void PWR_exitLPRunMode(void);

/**
 * @brief 
 * 
 * @param seconds 
 */
bool PWR_armRTC(uint16_t seconds, void (*on_cycle)());

void PWR_disarmRTC(void);

void PWR_wfi(void);

// /**
//  * Enter or exit Low Power Sleep Mode
//  *
//  * @param seconds    How long to go to sleep for
//  *
//  * @returns false if RTC isn't using LSE or LSI
//  */
// bool PWR_enterLPSleepMode(uint16_t seconds);

// /**
//  * Exit Low Power Sleep Mode
//  *
//  * @returns false if RTC isn't using LSE or LSI
//  */
// void PWR_exitLPSleepMode();

// /**
//  * By default, program will exit LPSleep upon interrupt.
//  * If you don't want that, call this function at the end of
//  * the interrupt.
//  * See RTC_WKUP_IRQHandler as example
//  *
//  * @param None
//  *
//  * @returns None
//  */
// void PWR_maintainLPSleep();

#endif /* SYSTEM_CONFIG_PWR_SLEEP_H_ */
