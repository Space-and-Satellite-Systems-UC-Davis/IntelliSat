#include "FreeRTOS.h"
#include "task.h"
#include "PWR/sleep.h"

/**
 * @brief Implements the port for vPortSuppressTicksAndSleep
 * 
 * @param xExpectedIdleTime The amount of ticks to be idle for
 */
void vPortSuppressTicksAndSleep(TickType_t xExpectedIdleTime);

