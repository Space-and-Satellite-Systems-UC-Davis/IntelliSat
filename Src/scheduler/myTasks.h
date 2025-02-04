#pragma once
#include "process.h"

/**
 * https://github.com/FreeRTOS/FreeRTOS-Kernel-Book/blob/main/ch05.md
 * https://www.freertos.org/Documentation/02-Kernel/06-Coding-guidelines/02-FreeRTOS-Coding-Standard-and-Style-Guide
 * https://www.freertos.org/Documentation/02-Kernel/04-API-references/06-Queues/00-QueueManagement
 */

// Include any structs and datatype to pass here
// * TODO Remove this demo struct

/* Define an enumerated type used to identify the source of the data. */
typedef enum
{
    eSender1,
    eSender2
} DataSource_t;

/* Define the structure type that will be passed on the queue. */
typedef struct
{
    uint8_t ucValue;
    DataSource_t eDataSource;
} Data_t;

// * End of the TODO

/* Declare two variables of type Data_t that will be passed on the queue. */
static const Data_t xStructsToSend[ 2 ] =
{
    { 100, eSender1 }, /* Used by Sender1. */
    { 200, eSender2 }  /* Used by Sender2. */
};

#define QUEUE_TICKS_TO_WAIT 100
#define TICKS_TO_BLOCK_SENDER 500



// Forward declaration of queues here
// You should initialize with xQueueCreate in main(), before calling xTaskCreate, vTaskStartScheduler
// Before using the Queue instances.
extern QueueHandle_t myQueue; 

void vSenderTask(void *pvParameters);

void vReceiverTask(void *pvParameter);
