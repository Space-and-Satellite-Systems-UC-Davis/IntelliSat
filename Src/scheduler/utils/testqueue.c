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

#include "testqueue.h"
#include "LED/led.h"
#include "print_scan.h"
#include "../../Src/system_config/Timers/timers.h"

#define led_delay 40000

void vSenderTask(void *pvParameters)
{
    BaseType_t taskStatus;
    TickType_t delay = QUEUE_TICKS_TO_WAIT / portTICK_PERIOD_MS;

    for(;;) {
        if (uxQueueSpacesAvailable(myQueue) > 0) {
            // Space is available, send the message
            taskStatus = xQueueSendToBack(myQueue, pvParameters, delay);
            if(taskStatus == pdPASS) {
                printMsg("Send to queue Successfully.\n");
                led_d2(1);
                nop(led_delay);
                led_d2(0);
            } else {
                printMsg("Could not send to the queue.\n");
            }
        } else {
            // Queue is full, so yield to allow the receiver to process
            printMsg("Sender: Queue full, waiting...\n");
            vTaskDelay(pdMS_TO_TICKS(QUEUE_TICKS_TO_WAIT));
        }
    }
}


void vReceiverTask(void *pvParameter) {
    Data_t xReceivedStructure;
    BaseType_t taskStatus;

    for(;;) {
        // Only process one message if the queue is full
        if (uxQueueMessagesWaiting(myQueue) == configTIMER_QUEUE_LENGTH) {
            taskStatus = xQueueReceive(myQueue, &xReceivedStructure, QUEUE_TICKS_TO_WAIT);
            if(taskStatus == pdPASS) {
                printMsg("Receiver: Processed message from %s = %d\n", 
                    (xReceivedStructure.eDataSource == eSender1) ? "Sender 1" : "Sender 2", 
                    xReceivedStructure.ucValue);
                led_d3(1);
                nop(led_delay);
                led_d3(0);
            }
        } else {
            vTaskDelay(pdMS_TO_TICKS(QUEUE_TICKS_TO_WAIT));
        }
    }
}
