#pragma once

#include "FreeRTOS.h"
#include "task.h"

/**
 * @brief FreeRTOS tickless idle hook.
 *
 * Called by the idle task when all other tasks are blocked.
 * Enters LP Run mode (or skips if already in system LP mode),
 * arms the RTC wakeup timer, executes WFI, then advances the
 * FreeRTOS tick count via vTaskStepTick using actual RTC elapsed
 * time — correct regardless of HCLK speed.
 *
 * @param xExpectedIdleTime Ticks the scheduler expects to be idle.
 */
void vPortSuppressTicksAndSleep(TickType_t xExpectedIdleTime);

/**
 * @brief Battery voltage monitor task.
 *
 * Polls pwrmon_getBusVoltage every 30 seconds via vTaskDelay.
 * Enters system low-power mode when voltage drops below
 * LP_MODE_ENTER_VOLTAGE_V, exits when it recovers above
 * LP_MODE_ENTER_VOLTAGE_V + LP_MODE_EXIT_HYSTERESIS_V.
 * Suspends and resumes ADCS task handles around transitions.
 */
void lowPwr_task(void *pvParameters);
