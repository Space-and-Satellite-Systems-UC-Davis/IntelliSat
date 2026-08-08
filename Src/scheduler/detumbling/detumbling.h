#ifndef DETUMBLING_H
#define DETUMBLING_H

#include "FreeRTOS.h"
#include "task.h"

void detumble_task(void *pvParameters);

// Use xTaskNotifyGive(detumbleHandle) to tell ADCS (vi_task_has_restarted())
// that detumbling was interrupted mid-execution.
extern TaskHandle_t detumbleHandle;

#endif
