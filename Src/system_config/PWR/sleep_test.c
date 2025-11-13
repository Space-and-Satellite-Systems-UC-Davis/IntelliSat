/*
 * sleep_test.c
 *
 *  Created on: Aug 12, 2025
 */

#include "sleep.h"
#include "RTC/rtc.h"
#include "WDG/watchdog.h"
#include <print_scan.h>

bool is_DBP_not_set() { return (PWR->CR1 & PWR_CR1_DBP) == 0; }
//Remember that using breakpoints may introduce delay
void testFunction_LPSleep() {
    printMsg("\r\nSleep Test Start. If you are seeing this multiple times, watchdog triggered reset\r\n");

	backup_domain_controlEnable();
	wait_with_timeout(is_DBP_not_set, DEFAULT_TIMEOUT_MS);
	RCC->BDCR |= RCC_BDCR_LSEON;
	backup_domain_controlDisable();

	// Window Watchdog not planned to be used
	WWDG->CR &= ~WWDG_CR_WDGA;

	rtc_config(LSE, true);
    printMsg("\r\nStart LPSleep, LSE, Expected 100 seconds\r\n");
    PWR_enterLPSleepMode(100);
    printMsg("\r\nLPSleep Over\n");

	rtc_config(LSI, true);
	// See http://efton.sk/STM32/gotcha/g200.html on LSI as RTC source
    printMsg("\r\nStart LPSleep, LSI, Expected 100 seconds +/- 3%%\r\n");
    PWR_enterLPSleepMode(100);
    printMsg("\r\nLPSleep Over\n");

	rtc_config(LSE, true);
    printMsg("\r\nStart LPSleep, LSE, Expected 300 seconds\r\n");
    PWR_enterLPSleepMode(300);
    printMsg("\r\nLPSleep Over\n");

    while(true) {
        printMsg("SUCCESS\r\n");
    }
}
