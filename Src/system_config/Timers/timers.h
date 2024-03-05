/*
 * timers.h
 */

#ifndef REALOP1_TIMERS_H_
#define REALOP1_TIMERS_H_

#include "stm32l476xx.h"
#include "../../globals.h"

//-----------------------------------------------------------------------------

#define TEST_TIMER       TIM2

#define EscControlTimer_ID 				4
#define EscControlTimer 				TIM4
#define EscControlTimer_ClockEnable() 	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM4EN
#define	EscControlTimer_ClockDisable() 	RCC->APB1ENR1 &= ~RCC_APB1ENR1_TIM4EN

#define PreemptTimer_ID					2
#define PreemptTimer     				TIM2
#define PreemptTimer_ClockEnable() 		RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN
#define	PreemptTimer_ClockDisable()		RCC->APB1ENR1 &= ~RCC_APB1ENR1_TIM2EN

#define BkgrndTimer_ID     				3
#define BkgrndTimer	     				TIM3
#define BkgrndTimer_ClockEnable() 		RCC->APB1ENR1 |= RCC_APB1ENR1_TIM3EN
#define	BkgrndTimer_ClockDisable() 		RCC->APB1ENR1 &= ~RCC_APB1ENR1_TIM3EN

#define ExpLogTimer_ID					6
#define ExpLogTimer      				TIM6
#define ExpLogTimer_ClockEnable() 		RCC->APB1ENR1 |= RCC_APB1ENR1_TIM6EN
#define ExpLogTimer_ClockDisable() 		RCC->APB1ENR1 &= ~RCC_APB1ENR1_TIM6EN

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

#define PWM_TIMER_ON()  EscControlTimer->CR1 |=  TIM_CR1_CEN;
#define PWM_TIMER_OFF() EscControlTimer->CR1 &= ~TIM_CR1_CEN;

/**
 * Configures the PWM timer registers to generate a signal at a given period.
 *
 * @param   period Period of the PWM signal, in microseconds. Note: Cannot be above 32K.
 * @returns Boolean to indicate if the initialization was successful
 */
bool init_pwm_timer(uint32_t period);
/**
 * Changes the Duty Cycle of the PWM signal... assumes the PWM is being generated already.
 * NOTE: Some percentges might not work properly due to integer math causing rounding errors
 * 
 * @param percent A value between 0-100 to indicate 
*/
void set_duty_cycle(uint8_t percent);

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

/**
 * Initializes the Heartbeat LED Timer (the Systck). 
 * Configures it to tick every ms.
*/
void heartbeat_init();


#endif	// REALOP1_TIMERS_H_
