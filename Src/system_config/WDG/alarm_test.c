/*
 * alarm_test.c
 *
 *  Created on: Sep 3, 2025
 */
#include "RTC/rtc.h"
#include "print_scan.h"

void do_something() { printMsg("Do something callback called \n\r"); }
void testFunction_RTC_Alarm() {
	printMsg("Starting RTC alarm test \n\r");

	rtc_setAlarm(1, false, do_something);

	while (true) {
		continue;
	}
}

