#include <globals.h>
#include "timers.h"
#include <LED/led.h>


// Global (external) variables and functions
extern int core_MHz;	// from core_config.h

#define HeartBeatTimer TIM6

void heartbeat_timer_init(void)
{
    // Enable the clock for TIM6
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM6EN;
    HeartBeatTimer->CR1 &= ~TIM_CR1_CEN; // Disable the timer
    HeartBeatTimer->CR1 |= TIM_CR1_URS;  // Only overflow/underflow generates an update interrupt
    HeartBeatTimer->CR1 |= ~TIM_CR1_DIR; // downcounting mode

    HeartBeatTimer->PSC = (core_MHz / 8) * 1000;
    HeartBeatTimer->ARR = 1000;
    HeartBeatTimer->DIER |= TIM_DIER_UIE; 

    NVIC_EnableIRQ(TIM6_DAC_IRQn);
    HeartBeatTimer->CR1 |= TIM_CR1_CEN;
}

void TIM6_DAC_IRQHandler(void)
{
    blinky(); // Toggle the heartbeat LED
}
