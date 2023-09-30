/*
 * timers.h
 */

#ifndef REALOP1_TIMERS_H_
#define REALOP1_TIMERS_H_

#include "stm32l476xx.h"

#define PWM_TIMER TIM4

void systick_init();

void timer_init();

#define PWM_TIMER_ON  PWM_TIMER->CR1 |=  TIM_CR1_CEN;
#define PWM_TIMER_OFF PWM_TIMER->CR1 &= ~TIM_CR1_CEN;
void pwm_timer_init();


#endif	// REALOP1_TIMERS_H_
