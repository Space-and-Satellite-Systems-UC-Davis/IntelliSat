/**
 * @file virtual_rtos.c
 * @author Danny Kwong cwkwong@ucdavis.edu
 * @brief 
 * @version 0.1
 * @date 2025-11-29
 * 
 * @author Jacob Tkeio jacobtkeio@gmail.com
 * @brief Implementation of critical sections for use in ADCS modes
 * @version 0.2
 * @date 2025-12-29
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include "scheduler/intelliTasks/intelliTasks_proto.h"
#include "virtual_rtos.h"
#include "FreeRTOS.h"
#include "task.h"

extern Taskhandle_t detumbleHandle;
extern Taskhandle_t experimentHandle;

typedef enum {
    VR_DETUMBLE,
    VR_EXPERIMENT
} vr_tasks;

void vr_enter_critical() {
    taskENTER_CRITICAL();
}

void vr_exit_critical() {
    taskEXIT_CRITICAL();
}

// TODO Should/how long to wait for notification
int vr_task_has_restarted(vr_tasks task) {
    switch (task) {
        case VR_DETUMBLE:
        ulTaskNotifyTake(detumbleHandle, 0)
        break;
        case VR_EXPERIMENT:
        ulTaskNotifyTake(experimentHandle, 0)
        break;
    }
}

void vr_delay_ms(int ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
}

void vr_get_curr_millis(uint64_t *curr_millis) {
    *curr_millis = (uint64_t)pdTICKS_TO_MS(xTaskGetTickCount());
}
