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

#define led_delay 40000

void vSenderTask(void *pvParameters)
{
    BaseType_t taskStatus;

    TickType_t delay = QUEUE_TICKS_TO_WAIT / portTICK_PERIOD_MS;
    const TickType_t block_delay = TICKS_TO_BLOCK_SENDER / portTICK_PERIOD_MS;

    for(;;) {
        TickType_t xLastWakeTime;
        xLastWakeTime = xTaskGetTickCount();
        if (uxQueueMessagesWaiting(myQueue) == configTIMER_QUEUE_LENGTH) {
            xTaskDelayUntil(&xLastWakeTime, block_delay);
            printMsg("Sender: Could not send to the queue since it is full!\r\n");
            // taskYIELD();
        }
        taskStatus = xQueueSendToBack(myQueue, pvParameters, delay);
        if(taskStatus != pdPASS) {
            printMsg("Could not send to the queue.\r\n");
            led_d1(1);
            pwm_setDutyCycle(20);
            nop(led_delay);
            led_d1(0);
        } else {
            printMsg("Send to queue Successfully.\r\n");
            led_d2(1);
            pwm_setDutyCycle(40);
            nop(led_delay);
            led_d2(0);
        }
        // taskYIELD();
    }
}


void vReceiverTask(void *pvParameter) {
    // TODO different types of Receivers maybe? Can avoid having to keep creating task for receivers
    // For The line below, any type of data allowed by FreeRTOS is allowed.
    // That means char, char *, TickType_t, BaseType_t, UBaseType_t, StackType_t, uints, and ints and so on.
    // As far as I understand even structs
    Data_t xReceivedStructure;

    BaseType_t taskStatus;

    for(;;) {
        if(uxQueueMessagesWaiting(myQueue) != configTIMER_QUEUE_LENGTH)              
        {
            printMsg("Receiver: Queue should have been full! %d messages in queue.\r\n", uxQueueMessagesWaiting(myQueue));
        }
        taskStatus = xQueueReceive(myQueue, &xReceivedStructure, QUEUE_TICKS_TO_WAIT);
        if(taskStatus == pdPASS )
        {
            // TODO Implement actual handler for when data is received from queue
            if(xReceivedStructure.eDataSource == eSender1) {
                printMsg("From Sender 1 = %d\r\n", xReceivedStructure.ucValue);
                led_d3(1);
                pwm_setDutyCycle(60);
                nop(led_delay);
                led_d3(0);
            } else {
                printMsg("From Sender 2 = %d\r\n", xReceivedStructure.ucValue);
                led_d4(1);
                pwm_setDutyCycle(80);
                nop(led_delay);
                led_d4(0);
            }
        }
        else
        {
            printMsg( "Could not receive from the queue.\r\n" );
            led_d5(1);
            pwm_setDutyCycle(100);
            nop(led_delay);
            led_d5(0);
        }
        // taskYIELD();
    }
}
