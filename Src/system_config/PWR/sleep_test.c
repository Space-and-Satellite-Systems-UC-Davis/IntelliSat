/*
 * sleep_test.c
 *
 *  Created on: Aug 12, 2025
 */

#include "sleep.h"
#include "RTC/rtc.h"
#include <print_scan.h>

void testFunction_LPSleep() {
	backup_domain_controlEnable();

	RCC->BDCR |= RCC_BDCR_LSEON;

	backup_domain_controlDisable();

//	watchdog_config(300);

	rtc_config(LSE, true);
    printMsg("\r\nStart LPSleep, LSE, Expected 100 seconds\r\n");
    PWR_enterLPSleepMode(100);
    printMsg("\r\nLPSleep Over\n");

	rtc_config(LSI, true);
    printMsg("\r\nStart LPSleep, LSI, Expected 100 seconds\r\n");
    PWR_enterLPSleepMode(100);
    printMsg("\r\nLPSleep Over\n");

	rtc_config(LSE, true);
    printMsg("\r\nStart LPSleep, LSE, Expected 300 seconds\r\n");
    PWR_enterLPSleepMode(300);
    printMsg("\r\nLPSleep Over\n");

    while(true) {
        printMsg("RAAAAAAAAAAAAAAAAAGH\r\n");
    }
}


