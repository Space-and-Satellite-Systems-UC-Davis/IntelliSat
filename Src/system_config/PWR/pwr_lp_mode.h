#pragma once

#include <stdbool.h>

/* Voltage thresholds — placeholder values, set before flight. */
#define LP_MODE_ENTER_VOLTAGE_V   3.5f
#define LP_MODE_EXIT_HYSTERESIS_V 0.2f

/**
 * True while the system is in battery-triggered low-power mode.
 * Read by the tickless idle hook to skip redundant LP Run enter/exit.
 */
extern volatile bool g_system_lp_mode_active;

/**
 * Enter system low-power mode:
 *   - Shuts down HDD, magnetorquer, IMU, magnetometer
 *   - Enters LP Run (80 MHz -> 1.25 MHz)
 *   - Sets g_system_lp_mode_active
 *
 * Must be called from task context only.
 * ADCS tasks must be suspended by the caller after this returns.
 */
void pwr_enterSystemLPMode(void);

/**
 * Exit system low-power mode:
 *   - Exits LP Run first (restores 80 MHz and TIM7 timing)
 *   - Re-initialises IMU and magnetometer
 *   - Restores HDD and magnetorquer power
 *   - Clears g_system_lp_mode_active
 *
 * Must be called from task context only.
 * ADCS tasks must be resumed by the caller after this returns.
 */
void pwr_exitSystemLPMode(void);
