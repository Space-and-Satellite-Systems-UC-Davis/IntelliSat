/*
 * alarm_test.c
 *
 *  Created on: Sep 3, 2025
 */
#include "RTC/rtc.h"
#include "print_scan.h"

void do_something1() { printMsg("Do something 1 \n\r"); }
void do_something2() { printMsg("Do something 2 \n\r"); }
void do_something3() { printMsg("Do something 3 \n\r"); }
void do_something4() { printMsg("Do something 3 \n\r"); }

void testFunction_RTC_Alarm() {
	printMsg("Starting RTC alarm test \n\r");

	rtc_scheduleCallback(5, 0, 0, false, do_something1);
	rtc_scheduleCallback(15, 0, 0, false, do_something2);
	rtc_scheduleCallback(30, 0, 0, false, do_something3);

	rtc_scheduleCallback(10, 0, 0, false, do_something4);

	while (true) {
		continue;
	}
}

