/*
 * pwm_timer.h
 */

#ifndef REALOP1_TIMERS_H_
#define REALOP1_TIMERS_H_

#include "stm32l476xx.h"
#include <globals.h>

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

#if OP_REV == 1

#define PWMTimer 					TIM4
#define PWMTimer_ClockEnable() 		RCC->APB1ENR1 |= RCC_APB1ENR1_TIM4EN
#define	PWMTimer_ClockDisable() 	RCC->APB1ENR1 &= ~RCC_APB1ENR1_TIM4EN

#elif OP_REV == 2 || OP_REV == 3

#define PWMTimer 					TIM2
void PWMTimer_ClockEnable();
void PWMTimer_ClockDisable();

#endif

void PWM_TIMER_ON();
void PWM_TIMER_OFF();
/**
 * Configures the PWM timer registers to generate a signal at a given period.
 *
 * @param   period Period of the PWM signal, in microseconds. Note: Cannot be above 32K.
 * @returns Boolean to indicate if the initialization was successful
 */
bool pwm_initTimer(uint32_t period);
/**
 * Changes the Duty Cycle of the PWM signal... assumes the PWM is being generated already.
 * NOTE: Some percentges might not work properly due to integer math causing rounding errors
 * 
 * @param percent A value between 0-100 to indicate 
*/
void pwm_setDutyCycle(uint8_t percent);

#endif	// REALOP1_TIMERS_H_