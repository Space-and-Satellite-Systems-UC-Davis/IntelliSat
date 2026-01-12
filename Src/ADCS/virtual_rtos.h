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
