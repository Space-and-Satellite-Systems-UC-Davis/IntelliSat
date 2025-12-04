#include "timers.h"

int count = 0;

void TIM8_IRQHandler()
{
    if (RTOSDebugTimer->SR & TIM_SR_UIF) {
        RTOSDebugTimer->SR &= ~TIM_SR_UIF;
        count++;
    }
}

int rtos_debug_timer_get_counter_value() {
    return count;   // just returns counter value
}

void rtos_debug_timer_init() {
    // systick timer period x 10 = .1 ms => R = 10000 Hz
    // R = F / [(ARR+1)(PSC+1)(RCR+1)]
    //      F   =  5 MHZ (5 000 000 HZ)
    //      R   =  10000 Hz
    //      RCR =  0


    uint32_t PSC = 0;       // ARR = 499, RCR = PSC = 0
    uint32_t ARR = 499;     // => R = 10000 = (5 x 10^6) / (500)
    uint32_t RCR = 0;


    RCC->APB2ENR |=  RCC_APB2ENR_TIM8EN;
    RTOSDebugTimer->CR1 &= ~TIM_CR1_CEN; // disable timer from CR1
    RTOSDebugTimer->EGR |= TIM_EGR_UG;

    RTOSDebugTimer->CR1 &= ~TIM_CR1_DIR; // 0 for up counter? no reason cuz just measures time
    RTOSDebugTimer->CR1 |= TIM_CR1_URS; // generate ug event
    RTOSDebugTimer->CR1 |= TIM_CR1_DIR;

    RTOSDebugTimer->DIER |= TIM_DIER_UIE; // generate interrupt for ug events
    NVIC_EnableIRQ(TIM8_IRQn);


    RTOSDebugTimer->PSC = PSC;
    RTOSDebugTimer->ARR = ARR;
    RTOSDebugTimer->RCR = RCR;

    RTOSDebugTimer->CR1 |= TIM_CR1_CEN;

}



