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
#include "stdbool.h"
#include <stdint.h>

// From FreeRTOS.h
typedef void * vi_task_handle_t;

/* -------------------------------------------------------------------------- */
/*                               HARDWARE MUTEX                               */
/* -------------------------------------------------------------------------- */
// TODO Better name for this??

/**
 * @brief Acquire the hardware mutex
 * the task are
 * Blocks until hardware is available. Uses priority inheritance.
 *
 * @param timeout_ms Maximum time to wait in milliseconds (0 = no wait,
 *                   UINT32_MAX = wait forever)
 * @return true if hardware acquired, false on timeout
 */
void vi_acquire_hardware(uint32_t timeout_ms);

/**
 * @brief Release exclusive access to hardware resources
 * Must be called by the same task that acquired the hardware
 */
void vi_release_hardware();

/* -------------------------------------------------------------------------- */
/*                              CRITICAL SECTION                              */
/* -------------------------------------------------------------------------- */

/**
 * @brief Enter critical section - disables interrupts and scheduler
 * Use before sending commands to actuators to ensure atomicity
 * MUST be paired with vi_exit_critical()
 */
void vi_enter_critcal();

/**
 * @brief Exit critical section - re-enables interrupts and scheduler
 */
void vi_exit_critical();

/* -------------------------------------------------------------------------- */
/*                              TASK COORDINATION                             */
/* -------------------------------------------------------------------------- */

/**
 * @brief Check if the current task should yield to a higher priority task
 * Call periodically at checkpoints
 *
 * @return true
 * @return false
 */
bool vi_should_yield();
/**
 * @brief Yield to a higher priority task and wait for resumption
 * releases mutex and blocks until signaled to resume
 *
 * @return true if successfully resumed
 * @return false on error
 */
bool vi_yield_wait_resume();

/**
 * @brief Signal a task to resume operation
 * Should be called by higher priority task when it finishes
 *
 * @param task_handle
 */
void vi_signal_task_resume(TaskHandle_t task_handle);

/* -------------------------------------------------------------------------- */
/*                             ACTUALTOR SHUTDOWN                             */
/* -------------------------------------------------------------------------- */

/**
 * @brief Safely shutdown all magnetorquers
 * Sets all coil current to zero atomically
 * Wraps around non atomic drivers code
 *
 * @return true on success
 * @return false on failure
 */
bool vi_safe_shutdown_magnetorquers();

/**
 * @brief Safely shutdown all reaction wheels
 * Sets all wheel speeds to zero atomically
 *
 * @return true on success, false on failure
 */
bool vi_safe_shutdown_reaction_wheels(void);

/**
 * @brief Safely shutdown all actuators
 * Turns off both magnetorquers and reaction wheels
 *
 * @return true on success, false on failure
 */
bool vi_safe_shutdown_all_actuators(void);

/* -------------------------------------------------------------------------- */
/*                              TASK NOTIFICATION                             */
/* -------------------------------------------------------------------------- */

/**
 * @brief Notify detumbling task (either start, stop, or resume)
 *
 * @param notification_value
 */
void vi_notify_detumbling_task(uint32_t notification_value);

/**
 * @brief Wait for notification with timeout
 *
 * @param timeout_ms Timeout in milliseconds
 * @param notification_value Output parameter for received notification
 * @return true if notification received
 * @return false on timeout
 */
bool vi_wait_for_notification(uint32_t timeout_ms,
                              uint32_t *notification_value);

/* -------------------------------------------------------------------------- */
/*                              NOTIFICATION BITS                             */
/* -------------------------------------------------------------------------- */

#define VI_NOTIFY_YIELD_REQUEST (1 << 0) // Higher priority task needs hardware
#define VI_NOTIFY_RESUME (1 << 1)        // Resume operation
#define VI_NOTIFY_SHUTDOWN (1 << 2)      // Shutdown requested
#define VI_NOTIFY_START (1 << 3)         // Start operation

/* -------------------------------------------------------------------------- */
/*                                   UTILITY                                  */
/* -------------------------------------------------------------------------- */
/**
 * @brief Check if the current task holds the hardware mutex
 * Useful for debugging
 *
 * @return true if the current task holds mutex
 * @return false otherwise
 */
bool vi_current_task_owns_hardware();

/**
 * @brief Get the handle.of the task currently holding the hardware mutex
 *
 * @return TaskHandle_t
 */
TaskHandle_t vi_get_hardware_owner();