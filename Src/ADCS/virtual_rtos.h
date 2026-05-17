/**
 * @file virtual_rtos.h
 * @author Danny Kwong (cwkwong@ucdavis.edu)
 * @brief Wrapper around FreeRTOS stuff for ADCS mode scheduling
 * @version 0.1
 * @date 2025-11-29
 *
 * @copyright Copyright (c) 2025
 *
 */
#pragma once

#include <stdint.h>


/**
 * @brief Enter critical section - disables interrupts and scheduler
 * Use before sending commands to actuators to ensure atomicity
 * MUST be paired with vi_exit_critical()
 */
void vi_enter_critical();


/**
 * @brief Exit critical section - re-enables interrupts and scheduler
 * If a higher-priority task wants to run, switch to it immediately.
 */
void vi_exit_critical();


/**
 * @brief Return true if the task has been restarted since the last
 * time it ran.
 */
int vi_task_has_restarted();


/**
 * @brief Sleep for ms milliseconds, yielding to the FreeRTOS scheduler.
 *
 * Uses vTaskDelay so the tickless idle hook can fire during the wait,
 * keeping elapsed time correct across LP Run clock speed changes.
 */
void vr_delay_ms(int ms);


/**
 * @brief Write the current FreeRTOS tick count converted to milliseconds.
 *
 * The tick count is advanced by vTaskStepTick via the RTC in the tickless
 * idle hook, so this remains accurate regardless of LP Run clock speed.
 */
void vr_get_curr_millis(uint64_t *curr_millis);
