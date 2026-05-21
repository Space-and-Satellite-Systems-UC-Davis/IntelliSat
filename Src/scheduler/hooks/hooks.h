#pragma once

#include "FreeRTOS.h"
#include "task.h"

void vApplicationMallocFailedHook( void );

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName);