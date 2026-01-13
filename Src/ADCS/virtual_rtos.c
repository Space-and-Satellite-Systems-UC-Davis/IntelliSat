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

void vi_enter_critical() {
    if (intelliTask_execMutex == NULL)
        intelliTask_execMutex = xSemaphoreCreateMutexStatic(&intelliTask_execMutexBuffer);

    (void)xSemaphoreTake(intelliTask_execMutex, portMAX_DELAY);
}

void vi_exit_critical() {
    if (xSemaphoreGive(intelliTask_execMutex) != pdTRUE) {
        // There is almost no recourse except waiting for the
        // hardware watchdog to restart the computer. TODO log error
        return;
    }
}
