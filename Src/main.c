/**
 * @file main.c
 * @brief Entrypoint of kernel systems
 *
 * Contains initial setup, and beginning of
 * main superloop for responsible for uninterrupted
 * runtime.
 *
 * @authors Nithin Senthil, Parteek Singh, Jacob Tkeo
 * @date 6/../23
 */

#include "platform_init.h"
#include <print_scan.h>
#include "system_config/LED/led.h"

/* Globals */
#include "globals.h"

/* File Includes */
#include "scheduler/include/intelliTasks.h"


#include "FreeRTOS.h"
#include "task.h"

#include <TestDefinition.h>
#define RUN_TEST	0	// 0 = run IntelliSat, 1 = run a very specific test
#define TEST_ID 	0	// ID of the test to run in case RUN_TEST = 1

/* Macros */
#define SYSTICK_DUR_U 10000          // Config. of systick timer in usec (1 ms)
#define BATTERY_THRESHOLD 20        // TODO: Min. battery voltage value, below which mode -> CHARGING

static const led_task_struct blinkLedTwo = { .led_num = 2, .delay_ms = 100000 };
static const led_task_struct blinkLedThree = { .led_num = 3, .delay_ms = 100000 };

/**
 * @brief Superloop
 *
 * Contains initial timer and interrupt handler (TESTING) and
 * the main superloop. Serves as standard behavior when
 * there is no scheduler intervention.
 */
int branch_main() {
    printMsg("watchdog status: %d", (RCC->CSR >> 29) & 1);
    xTaskCreate(led_task, "LED_blink_2", 400, (void*)&blinkLedTwo, configMAX_PRIORITIES-3, NULL);
    xTaskCreate(led_task, "LED_blink_3", 400, (void*)&blinkLedThree, configMAX_PRIORITIES-3, NULL);
    // xTaskCreate(blinkD2, "LED_blink_2", 128, (void*)1 , configMAX_PRIORITIES-3, NULL);

    vTaskStartScheduler();

    while(1) {}

    return 0;

}

#define RUN_TEST	0	// 0 = run IntelliSat, 1 = run a very specific test
#define TEST_ID 	0	// ID of the test to run in case RUN_TEST = 1

#include <TestDefinition.h>

int main() {
    init_init();
    init_platform(!RUN_TEST);
    // ^ don't want to run the Scheduler in case we are running other tests
    printMsg("RESTART\r\n");
    
    //TODO: use RTC first_time flag.
    //if (first_time) {
        init_first_time();
    //}
    
    
    #if (RUN_TEST==1) && (TEST_ID != 0)

    void (*testFunc)();
    testFunc = getTestFunction(TEST_ID);
    testFunc();

    #else
    printMsg("watchdog status: %d", (RCC->CSR >> 29) & 1);
//     RCC->CSR |= (1 << 23);
    branch_main();
    #endif
}
