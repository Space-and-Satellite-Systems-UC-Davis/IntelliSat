/*
 * timers.h
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
#define PWMTimer_ClockEnable() 		RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN
#define	PWMTimer_ClockDisable() 	RCC->APB1ENR1 &= ~RCC_APB1ENR1_TIM2EN

#endif

#define PWM_TIMER_ON()  PWMTimer->CR1 |=  TIM_CR1_CEN;
#define PWM_TIMER_OFF() PWMTimer->CR1 &= ~TIM_CR1_CEN;

#define SYSTICK_CTRL_COUNTER 1

enum pwm_timer_cap_comp_1_modes {
    TIM_CCMR1_OC1M_FROZEN,
    TIM_CCMR1_OC1M_SET_CHAN_1_ACTIVE_ON_MATCH,
    TIM_CCMR1_OC1M_SET_CHAN_1_INACTIVE_ON_MATCH,
    TIM_CCMR1_OC1M_TOGGLE,
    TIM_CCMR1_OC1M_FORCE_INACTIVE,
    TIM_CCMR1_OC1M_FORCE_ACTIVE,
    TIM_CCMR1_OC1M_PWM_MODE_1,
    TIM_CCMR1_OC1M_PWM_MODE_2,
    TIM_CCMR1_OC1M_RETRIGGERABLE_OPM_MODE_1,
    TIM_CCMR1_OC1M_RETRIGGERABLE_OPM_MODE_2,
    TIM_CCMR1_OC1M_COMBINED_PWM_MODE_1 = 0xc,
    TIM_CCMR1_OC1M_COMBINED_PWM_MODE_2 = 0xd,
    TIM_CCMR1_OC1M_ASYMMETRIC_PWM_MODE_1 = 0xe,
    TIM_CCMR1_OC1M_ASYMMETRIC_PWM_MODE_2 = 0xf,
};

/**
 * Configures the PWM timer registers to generate a signal at a given period.
 *
 * @param   period Period of the PWM signal, in microseconds. Note: Cannot be above 32K.
 * @returns Boolean to indicate if the initialization was successful
 */
bool pwm_initTimer(uint32_t period);
/**
 * Changes the Duty Cycle of the PWM signal... assumes the PWM is being generated already.
 * NOTE: Some percentages might not work properly due to integer math causing rounding errors
 * 
 * @param percent A value between 0-100 to indicate 
*/
void pwm_setDutyCycle(uint8_t percent);

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

/**
 * Initializes the Heartbeat LED Timer (the Systick). 
 * Configures it to tick every ms.
 *
 * @param run_scheduler If set to true, the IntelliSat Scheduler will be
 *        call by the Systick
 * @returns none
*/
void systick_init(bool run_scheduler);

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

#define ExpLogTimer      				TIM6
#define ExpLogTimer_ClockEnable() 		RCC->APB1ENR1 |= RCC_APB1ENR1_TIM6EN
#define ExpLogTimer_ClockDisable() 		RCC->APB1ENR1 &= ~RCC_APB1ENR1_TIM6EN

#define logger_expTimerOn()  ExpLogTimer->CR1 |=  TIM_CR1_CEN;
#define logger_expTimerOff() ExpLogTimer->CR1 &= ~TIM_CR1_CEN;

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

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

#define StartupTimer					TIM5
#define StartupTimer_ClockEnable()		RCC->APB1ENR1 |=  RCC_APB1ENR1_TIM5EN
#define StartupTimer_ClockDisable()		RCC->APB1ENR1 &= ~RCC_APB1ENR1_TIM5EN

/**
 * Configures the Startup Timer registers to generate
 * an interrupt in 30 minutes
 *
 * @returns Boolean to indicate if the initialization was successful
 */
void timer_waitStartupTime();


#endif	// REALOP1_TIMERS_H_
