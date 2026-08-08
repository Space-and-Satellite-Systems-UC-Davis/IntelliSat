/**
 * @file virtual_rtos.c
 * @author Danny Kwong cwkwong@ucdavis.edu
 * @author Jacob Tkeio jacobtkeio@gmail.com
 * @brief Implementation of critical sections for use in ADCS modes
 * @version 0.4
 * @date 2025-11-29
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "virtual_rtos.h"
#include "FreeRTOS.h"
#include "task.h"
#include <globals.h>

#define ADCS_LOOP_STALE_TOLERANCE_MS 100

extern TaskHandle_t detumbleHandle;
extern TaskHandle_t experimentHandle;

static volatile uint64_t detumbleLastCheckMs = 0;
static volatile uint64_t experimentLastCheckMs = 0;

void vi_enter_critical() {
    taskENTER_CRITICAL();
}

void vi_exit_critical() {
    taskEXIT_CRITICAL();
}

int vi_task_has_restarted() {
    TaskHandle_t current = xTaskGetCurrentTaskHandle();
    volatile uint64_t *last;
    if (current == detumbleHandle) {
        last = &detumbleLastCheckMs;
    } else if (current == experimentHandle) {
        last = &experimentLastCheckMs;
    } else {
        return 0;
    }
    uint64_t now = getSysTime();
    uint64_t prev = *last;
    *last = now;
    return (prev != 0) && ((now - prev) > ADCS_LOOP_STALE_TOLERANCE_MS);
}

void vi_reset_stale_timer() {
    TaskHandle_t current = xTaskGetCurrentTaskHandle();
    if (current == detumbleHandle) {
        detumbleLastCheckMs = 0;
    } else if (current == experimentHandle) {
        experimentLastCheckMs = 0;
    }
}
