#include "globals.h"
#include "FreeRTOS.h"
#include "print_scan.h"
#include "task.h"
#include "LED/led.h"

static bool led2_state=true;
static bool led3_state=true;
static bool led4_state=true;
static bool led5_state=true;


typedef struct {
    uint32_t delay_ms;
    //! Should be between 2 and 5 inclusive
    uint8_t led_num;
} ledTask_struct;

void blinkLed(void * pvParameters) {
    ledTask_struct * param = (ledTask_struct*) pvParameters;
    for(;;) {
        switch (param->led_num) {
            case 2:
                led_d2(led2_state);
                led2_state = !led2_state;
                vTaskDelay(pdMS_TO_TICKS(param->delay_ms));
                continue;
            case 3:
                led_d3(led3_state);
                led3_state = !led3_state;
                vTaskDelay(pdMS_TO_TICKS(param->delay_ms));
                continue;
            case 4:
                led_d4(led4_state);
                led4_state = !led4_state;
                vTaskDelay(pdMS_TO_TICKS(param->delay_ms));
                continue;
            case 5:
                led_d5(led5_state);
                led5_state = !led5_state;
                vTaskDelay(pdMS_TO_TICKS(param->delay_ms));
                continue;
            case 1:
                printMsg("The heart beat led (d1) is solely reserved for blinky\n");
                continue;
            default:
                printMsg("No valid cases\n");
                vTaskDelay(0); // Should allow context switch
        }
    }
}
