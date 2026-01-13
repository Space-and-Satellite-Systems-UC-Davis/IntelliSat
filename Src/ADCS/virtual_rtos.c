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
#include "scheduler/intelliTask.h"

static int task_restarts[TASK_TABLE_LEN];
static TaskHandle_t last_task = NULL;


int get_idx_of_handle(TaskHandle_t handle) {
    for (int i = 0; i < TASK_TABLE_LEN; i++)
        if (task_table[i].FreeRTOS_handle == handle)
            return i;
}

void vi_enter_critical() {
    // Lazy mutex creation
    if (intelliTask_execMutex == NULL)
        intelliTask_execMutex = xSemaphoreCreateMutexStatic(&intelliTask_execMutexBuffer);

    // If swapping tasks, save the fact that the previous task restarted
    // TODO move this logic somewhere guaranteed to run on a switch
    if (last_task && xTaskGetCurrentTaskHandle() != last_task)
        task_restarts[get_idx_of_handle(last_task)] = 1;

    (void)xSemaphoreTake(intelliTask_execMutex, portMAX_DELAY);
}

void vi_exit_critical() {
    last_task = xTaskGetCurrentTaskHandle();

    if (xSemaphoreGive(intelliTask_execMutex) != pdTRUE) {
        // There is almost no recourse except waiting for the
        // hardware watchdog to restart the computer. TODO log error
        return;
    }
}

int vi_task_has_restarted() {
    int idx = get_idx_of_handle(xTaskGetCurrentTaskHandle());
    int has_restarted = task_restarts[idx];
    task_restarts[idx] = 0;
    return has_restarted;
}
