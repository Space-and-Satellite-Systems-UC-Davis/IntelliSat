#include <globals.h>
#include "timers.h"
#include <LED/led.h>

#define HeartBeatTimer TIM7

static volatile uint64_t time = 0;

void delay_ms(uint64_t ms) {
    uint64_t start_time = getSysTime();
    while (getSysTime() - start_time < ms);
}

// Cortex-M4 cannot atomically read a 64-bit ISR-owned counter.
uint64_t getSysTime() {
    uint32_t mask = __get_PRIMASK();
    __disable_irq();
    uint64_t snapshot = time;
    __set_PRIMASK(mask);
    return snapshot;
}

bool heartbeat_timer_init()
{
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM7EN; //Enable clock for TIM7
    HeartBeatTimer->CR1 &= ~TIM_CR1_CEN; //Disable timer from control register
    HeartBeatTimer->CR1 |= TIM_CR1_URS; //Generate UG event on rollovers
    HeartBeatTimer->CR1 |= TIM_CR1_DIR; //downcounting

    //1Khz timer, 1 ms period
    HeartBeatTimer->PSC = 99; 
    HeartBeatTimer->ARR = 99;
    HeartBeatTimer->EGR = TIM_EGR_UG; // Load PSC/ARR before starting the first tick.
    HeartBeatTimer->SR &= ~TIM_SR_UIF;

    HeartBeatTimer->DIER |= TIM_DIER_UIE; // Enable update interrupt

    NVIC_EnableIRQ(TIM7_IRQn); // Enable TIM7 interrupt
    NVIC_SetPriority(TIM7_IRQn, 4);
    HeartBeatTimer->CR1 |= TIM_CR1_CEN; // Enable timer
    
    return true;
}

void TIM7_IRQHandler()
{
    if (HeartBeatTimer->SR & TIM_SR_UIF) {
        HeartBeatTimer->SR &= ~TIM_SR_UIF;
        time++;
        blinky();
    }
}
