#include "timers.h"
#include <print_scan.h>
#include <globals.h>
#include <LED/led.h>

// 9600
// otherwise default



void delay_s(int s) {
	int start_time = rtos_debug_timer_get_counter_value();
	while (rtos_debug_timer_get_counter_value() - start_time < (s*10000));
}

void testFunction_rtos_debug_timer() {
     
    rtos_debug_timer_init();
    // volatile int counter_value = 0;

    while (1) {

        // int temp = counter_value;
        // counter_value = rtos_debug_timer_get_counter_value();
        // int diff = counter_value - temp;

        // counter_value = 100;
        // NVIC_DisableIRQ(TIM8_IRQn);
        // printMsg("diff: %d\r\n", diff);
        led_d2(1);
        // NVIC_EnableIRQ(TIM8_IRQn);

        delay_s(1);
        led_d2(0);
        delay_s(1);
    }
}


