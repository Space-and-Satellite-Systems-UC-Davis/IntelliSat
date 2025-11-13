#include "radio_task.h"

#define QUEUE_TICKS_TO_WAIT 100
#define MAX_TRIES 5
#define RADIO_BUS USART1

struct ExperimentLog
{
    char ucMessageID;
    char buffer[20];
};

void radio_queue_handler(void *pvParameters){

    //queue data struct definition in experiment_logger
    ExperimentLog experimentData;
    BaseType_t taskStatus;
    TickType_t delay = QUEUE_TICKS_TO_WAIT / portTICK_PERIOD_MS;
    int retryCount = 0;

    for(;;)
    {
        if(uxQueueMessagesWaiting(myQueue)){
            retryCount = 0;
            taskStatus = xQueueReceive(myQueue, &experimentData, QUEUE_TICKS_TO_WAIT);
            if(taskStatus == pdPASS) {

                // Transmit without header
                while(retryCount < MAX_TRIES && !crc_transmit(RADIO_BUS, (uint8_t*)&experimentData, sizeof(struct ExperimentLog))) {
                    retryCount++;
                }
                if(retryCount == MAX_TRIES){ //NACK Transmission failed
                    printMsg("Radio: PFC->RADIO transmission failed");
                    printMsg(experimentData);
                }
            }
        } 
        else{
            vTaskDelay(pdMS_TO_TICKS(QUEUE_TICKS_TO_WAIT));
        }
    }
}
