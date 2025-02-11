/**
 * @file tasks.c
 * @brief All task related methods using FreeRTOS
 *
 * Contains methods pertaining Sender and Receiver task.
 * All references can be found in tasks.h.
 *
 * @authors Danny Kwong
 * @date Jan 14th, 2024
 */

#include "myTasks.h"
#include "../../Src/system_config/LED/led.h"
#include "../tools/print_scan.h"
#include "../../Src/system_config/Timers/timers.h"

#define led_delay 10000

#define TASK_1_BIT (1UL << 0UL)
#define TASK_2_BIT (1UL << 1UL)

void setter(void *pvParameters) {

    // Blink to indicate task start
    led_d5(true);
     vTaskDelay(pdMS_TO_TICKS(2000));
    led_d5(false);

    led_d3(true);
     vTaskDelay(pdMS_TO_TICKS(3000));

    (void)xEventGroupSetBits(events, (const EventBits_t) 0x1);
    led_d3(false);

    vTaskDelete(NULL);
}


void reciever(void *pvParameters) {

    // Blink to indicate task start
    led_d1(true);
    vTaskDelay(pdMS_TO_TICKS(2000));
    led_d1(false);
    
    // Starting wait
    led_d2(true);

    (void)xEventGroupWaitBits(
        events,
        (const EventBits_t) 0x1,
        true, //clear bit on exit
        false, //wait for all bits in bit mask to be true
        portMAX_DELAY);

    // Delay to simulate work after unblocked
     vTaskDelay(pdMS_TO_TICKS(1000));
    
    led_d2(false);

    vTaskDelete(NULL);
}
