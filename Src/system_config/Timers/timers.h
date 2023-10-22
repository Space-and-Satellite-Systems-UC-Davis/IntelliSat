/*
 * timers.h
 */

#ifndef REALOP1_TIMERS_H_
#define REALOP1_TIMERS_H_

#include "stm32l476xx.h"
#include "../../globals.h"

//-----------------------------------------------------------------------------

#define TEST_TIMER       TIM2

#define EscControlTimer_ID 				1
#define EscControlTimer 				TIM1
#define EscControlTimer_ClockEnable() 	RCC->APB2ENR |= RCC_APB1ENR_TIM1EN
#define	EscControlTimer_ClockDisable() 	RCC->APB2ENR &= ~RCC_APB1ENR_TIM1EN

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

#define PWMTimer_ID						4
#define PWMTimer         				TIM4
#define PWMTimer_ClockEnable() 			RCC->APB1ENR1 |= RCC_APB1ENR1_TIM4EN
#define PWMTimer_ClockDisable() 		RCC->APB1ENR1 &= ~RCC_APB1ENR1_TIM4EN

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

bool register_timer(int timer, void* (handler_func)());
bool configure_timer(TIM_TypeDef* tim, int units, uint16_t amount);

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

#define PWM_TIMER_ON()  PWMTimer->CR1 |=  TIM_CR1_CEN;
#define PWM_TIMER_OFF() PWMTimer->CR1 &= ~TIM_CR1_CEN;
bool init_pwm_timer(uint32_t period);
void set_duty_cycle(uint8_t percent);

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

void heartbeat_init();

//-----------------------------------------------------------------------------


#endif	// REALOP1_TIMERS_H_
