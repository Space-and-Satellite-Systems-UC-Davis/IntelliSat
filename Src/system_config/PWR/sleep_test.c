/*
 * sleep_test.c
 *
 *  Created on: Aug 12, 2025
 */

#include "sleep.h"
#include <print_scan.h>

void testFunction_LPSleep() {
    printMsg("Start LPSleep\n");
    watchdog_config(9999999);

    PWR_enterLPSleepMode(100);

    printMsg("LPSleep Over\n");

    while(true) {
    	continue;
    }
}


