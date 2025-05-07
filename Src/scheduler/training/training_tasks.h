#include "scheduler/schedulerGlobals.h"
#include "FreeRTOS.h"
#include "task.h"

/**
 * ! the above starting handers are what you'll usuaully work with in schedulers (and more).
 * You will need the apprprate system header (e.g. math.h) for the accosiated functions
 * There are other specific includes that you should use should you want certain FreeRTOS feautres. (e.g. to delay a task)
 * You would also need to include the correct REALOP headers if you want certain drivers (e.g. led)
 * Look at other files (e.g.main.c) for how to include the led header.
 */

static bool led1_state = 0;
static bool led2_state = 0;
static bool led3_state = 0;
static bool led4_state = 0;
static bool led5_state = 0;
const long long led_delay = 400;

void toggle_LED(u_int8_t num)
{
    switch (num)
    {
    case 1:
        led_d1(!led1_state);
        // Same as led1_state = !led1_state
        led1_state ^= 1;
        break;
    case 2:
        led_d2(!led2_state);
        led2_state ^= 1;
        break;
    case 3:
        led_d3(!led3_state);
        led3_state ^= 1;
        break;
    case 4:
        led_d4(!led4_state);
        led4_state ^= 1;
        break;
    case 5:
        led_d5(!led5_state);
        led5_state ^= 1;
        break;
    default:
        break;
    }
}

void blink_LED(u_int8_t num)
{
    toggle_LED(num);
    nop(led_delay);
    toggle_LED(num);
}

// Your custom task should go here.

void vLedPattern1Task(void *pvParameters)
{
    while (true)
    {
        toggle_LED(1);
        toggle_LED(3);
        toggle_LED(5);
        // x / portTICK_PERIOD_MS gives you x milliseconds
        // Look into FreeRTOS/Source/tasks.c for details on vTaskDelay
        // Or consult the freeRTOS documentation
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void vLedPattern2Task(void *pvParameters)
{
    while (true)
    {
        toggle_LED(1);
        toggle_LED(2);
        // x / portTICK_PERIOD_MS gives you x milliseconds
        // Look into FreeRTOS/Source/tasks.c for details on vTaskDelay
        // Or consult the freeRTOS documentation
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}