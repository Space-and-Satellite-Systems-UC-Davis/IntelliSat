/**
 * @file watchdog.h
 * @brief Periodically pull up and tear down scheduler tasks.
 *
 * @author Jacob Tkeio
 * @date 1/27/2025
 */

#ifndef SCHEDULER_WATCHDOG_H_
#define SCHEDULER_WATCHDOG_H_

#include "FreeRTOS.h"
#include "task.h"

void watchdog(void *args);

#endif /* SCHEDULER_WATCHDOG_H_ */
