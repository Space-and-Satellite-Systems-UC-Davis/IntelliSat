/*
 * timers.h
 */

#ifndef REALOP1_TIMERS_H_
#define REALOP1_TIMERS_H_

#include "stm32l476xx.h"
#include <globals.h>

#define ExpLogTimer      				TIM6
void ExpLogTimer_ClockEnable();
void ExpLogTimer_ClockDisable();

void logger_expTimerOn();
void logger_expTimerOff();

/**
 * Configures the Experiment Logger timer registers to generate
 * an interrupt every 100 ms.
 *
 * @returns Boolean to indicate if the initialization was successful
 */
bool logger_initExpTimer();

/*
 * Binds a function to the log timer's interrupt handler.
 * Essentially lets the user decide which function to run when the log timer
 * interrupts.
 *
 * @param   A pointer to the interrupt handler function. The function must be
 *          void and take no inputs
 * @returns none
 */
void logger_registerLogFunction(void (*func)());

#endif	// REALOP1_TIMERS_H_