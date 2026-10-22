#include "sleep_primitives.h"

typedef enum OperatingMode {
	RUN, LPRUN, LPSLEEP
} OperatingMode;

/**
 * Enter and then exit Low Power Sleep Mode
 *
 * @param seconds    How long to go to sleep for
 *
 * @returns false if RTC isn't using LSE or LSI
 */
bool PWR_enterLPSleepMode(uint16_t seconds);

/**
 * By default, program will exit LPSleep upon interrupt.
 * If you don't want that, call this function at the end of
 * the interrupt.
 * See RTC_WKUP_IRQHandler as example
 *
 * @param None
 *
 * @returns None
 */
void PWR_maintainLPSleep();