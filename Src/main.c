#include <print_scan.h>
#include "platform_init.h"
#include "scheduler/ledtask.h"
#include "scheduler/testqueue.h"
#include "queue.h"
#include "stm32l476xx.h"

#define RUN_TEST	0	// 0 = run IntelliSat, 1 = run a very specific test
#define TEST_ID 	0	// ID of the test to run in case RUN_TEST = 1

#include <TestDefinition.h>

static ledTask_struct led2task_t = {1000,2};

static ledTask_struct led3task_t = {1000,3};

volatile QueueHandle_t myQueue;

int branch_main() {
    //TODO: use RTC first_time flag.
    //if (first_time) {
    //  init_first_time()
    //}
//    myQueue = xQueueCreate(configTIMER_QUEUE_LENGTH, sizeof(Data_t));
//    vQueueAddToRegistry(myQueue, "Testing Queue");

//    xTaskCreate(vSenderTask, "Sender1", 1000, &(xStructsToSend[ 0 ]), configMAX_PRIORITIES-1, NULL);
    // xTaskCreate(vSenderTask, "Sender2", 1000, &( xStructsToSend[ 1 ]), configMAX_PRIORITIES-1, NULL);

    // The sender task should always be prioritized over the receiver
//    xTaskCreate(vReceiverTask, "Receiver", 1000, NULL, configMAX_PRIORITIES-1, NULL );

     xTaskCreate(blinkLed, "blink_led_2", 128, (void *)&led2task_t, configMAX_PRIORITIES - 3, NULL);
    xTaskCreate(blinkLed, "blink_led_3", 128, (void *)&led3task_t, configMAX_PRIORITIES - 3, NULL);

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
