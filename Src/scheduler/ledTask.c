#include "include/intelliTasks.h"
#include "print_scan.h"
#include <LED/led.h>

void led_task(void*pvParameters) {
    led_task_struct params = * (led_task_struct*) pvParameters;
    switch (params.led_num) {
        case 1:
            break;
        case 2:
            led_d2(true);
            vTaskDelay(pdMS_TO_TICKS(params.delay_ms));
            led_d2(false);
            vTaskDelay(pdMS_TO_TICKS(params.delay_ms));
            break;
        case 3:
            led_d3(true);
            vTaskDelay(pdMS_TO_TICKS(params.delay_ms));
            led_d3(false);
            vTaskDelay(pdMS_TO_TICKS(params.delay_ms));
            break;
        case 4:
            led_d4(true);
            vTaskDelay(pdMS_TO_TICKS(params.delay_ms));
            led_d4(false);
            vTaskDelay(pdMS_TO_TICKS(params.delay_ms));
            break;
        case 5:
            led_d5(true);
            vTaskDelay(pdMS_TO_TICKS(params.delay_ms));
            led_d5(false);
            vTaskDelay(pdMS_TO_TICKS(params.delay_ms));
            break;
        default:
            printMsg("Not a valid case");
            break;
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
}

void blinkD2( void *pvParameters ) {
	volatile uint64_t del = 1000;
    for( ;; )
    {
        // Toggle the LED
        // HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
        for(int i = 0; i < 5; i++){
            led_d2(true);
            delay_ms(del);
            led_d2(false);
            delay_ms(del);
        }
        vTaskDelay(500);
    }
}
