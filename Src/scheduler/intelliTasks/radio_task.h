#ifndef RADIO_TASK_H
#define RADIO_TASK_H

#include "globals.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <UART/crc.h>


void radio_queue_handler(void *pvParameters);

extern volatile QueueHandle_t myQueue;

#endif