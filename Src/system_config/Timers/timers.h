/*
 * timers.h
 */

#ifndef REALOP1_TIMERS_H_
#define REALOP1_TIMERS_H_

#include "stm32l476xx.h"
#include <globals.h>

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

/**
 * Initializes the Heartbeat LED Timer (the Systck). 
 * Configures it to tick every ms.
 *
 * @param run_scheduler If set to true, the IntelliSat Scheduler will be
 *        call by the Systick
 * @returns none
*/
void systick_init(bool run_scheduler);//systemtimer, platform_init

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-


#define StartupTimer					TIM5
void StartupTimer_ClockEnable();
void StartupTimer_ClockDisable();

/**
 * Configures the Startup Timer registers to generate
 * an interrupt in 30 minutes
 *
 * @returns Boolean to indicate if the initialization was successful
 */
void timer_waitStartupTime();//startuptimer.c, platform_init.c

#endif	// REALOP1_TIMERS_H_
