/*
 * sensor_monitor.h
 *
 * Streams sensor readings on the debug console as CSV, for flatsat testing.
 * Press a key in the serial terminal to choose what gets printed (h for help).
 */

#ifndef REALOP1_SENSOR_MONITOR_H_
#define REALOP1_SENSOR_MONITOR_H_

/**
 * Resets the monitor to print every channel. Call after init_platform().
 *
 * @param none
 *
 * @returns none
 */
void sensor_monitor_init();

/**
 * Handles pending console keys and SW1, and prints a CSV line when one is due.
 * Call this repeatedly.
 *
 * @param none
 *
 * @returns none
 */
void sensor_monitor_poll();

/**
 * Test entry point: runs the monitor forever.
 */
void testFunction_Sensor_Monitor();

#endif // REALOP1_SENSOR_MONITOR_H_
