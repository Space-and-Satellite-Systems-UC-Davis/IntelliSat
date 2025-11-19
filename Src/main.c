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

Data_t xStructsToSend[2] = {
    { 100, eSender1 }, /* Used by Sender1. */
    { 200, eSender2 }  /* Used by Sender2. */
};

int branch_main() {
    //TODO: use RTC first_time flag.
    // if (first_time) {
    //     init_first_time()
    // }
    size_t heapBefore = xPortGetFreeHeapSize();
    printMsg("[DEBUG] Initial free heap: %u bytes\n", heapBefore);
    // ------------------------------ //
    myQueue = xQueueCreate(configTIMER_QUEUE_LENGTH, sizeof(Data_t));

    if (myQueue == NULL) {
        printMsg("Queue creation FAILED!\n");
        while(1);
    }
    // Check heap after queue creation
    size_t heapAfterQueue = xPortGetFreeHeapSize();
    printMsg("[DEBUG] Heap after queue: %u bytes (used %u)\n", 
             heapAfterQueue, heapBefore - heapAfterQueue);

    vQueueAddToRegistry(myQueue, "Testing Queue");
    // ------------------------------ //
    xTaskCreate(vSenderTask, "Sender1", 256, (void * ) &(xStructsToSend[ 0 ]), configMAX_PRIORITIES-1, NULL);
    
    size_t heapAfterTask1 = xPortGetFreeHeapSize();
    printMsg("[DEBUG] Heap after Sender1: %u bytes (used %u)\n", heapAfterTask1, heapAfterQueue - heapAfterTask1);
    // ------------------------------ //
    BaseType_t result2 = xTaskCreate(vSenderTask, "Sender2", 256, (void *)&(xStructsToSend[1]), configMAX_PRIORITIES-1, NULL);
    xTaskCreate(vSenderTask, "Sender2", 256, (void * ) &( xStructsToSend[ 1 ]), configMAX_PRIORITIES-1, NULL);

    printMsg("[DEBUG] Sender2 created successfully\n");
    
    size_t heapAfterTask2 = xPortGetFreeHeapSize();
    printMsg("[DEBUG] Heap after Sender2: %u bytes (used %u)\n", heapAfterTask2, heapAfterTask1 - heapAfterTask2);
    // ------------------------------ //
    // The sender task should always be prioritized over the receiver
    xTaskCreate(vReceiverTask, "Receiver", 256, NULL, configMAX_PRIORITIES-1, NULL );

    size_t heapAfterTask3 = xPortGetFreeHeapSize();
    printMsg("[DEBUG] Heap after Receiver: %u bytes (used %u)\n", heapAfterTask3, heapAfterTask2 - heapAfterTask3);
    // ------------------------------ //
//    xTaskCreate(blinkLed, "blink_led_2", 128, (void *)&led2task_t, configMAX_PRIORITIES - 3, NULL);
//    xTaskCreate(blinkLed, "blink_led_3", 128, (void *)&led3task_t, configMAX_PRIORITIES - 3, NULL);

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
