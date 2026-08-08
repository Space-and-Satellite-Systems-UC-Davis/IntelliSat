#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include "FreeRTOS.h"
#include "task.h"

void experiment_task(void *pvParameters);

// Use xTaskNotifyGive(experimentHandle) to tell ADCS (vi_task_has_restarted())
// that the experiment was interrupted mid-execution.
extern TaskHandle_t experimentHandle;

#endif
