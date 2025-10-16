#include <print_scan.h>
#include "platform_init.h"
#include "scheduler/ledtask.h"

#define RUN_TEST	0	// 0 = run IntelliSat, 1 = run a very specific test
#define TEST_ID 	0	// ID of the test to run in case RUN_TEST = 1

#include <TestDefinition.h>

static ledTask_struct led2task_t = {1000,2};

int branch_main() {
    //TODO: use RTC first_time flag.
    //if (first_time) {
    //  init_first_time()
    //}
    xTaskCreate(blinkLed, "blink_led_2", 128, (void *)&led2task_t, configMAX_PRIORITIES - 3, NULL);

    vTaskStartScheduler();

    while (1) {
        continue;
    }
    return 0;
}

int main() {
    init_init();
    init_platform();
    #if (RUN_TEST==1) && (TEST_ID != 0)

        void (*testFunc)();
        testFunc = getTestFunction(TEST_ID);
        testFunc();

    #else
        branch_main();
    #endif
    return 0;
}
