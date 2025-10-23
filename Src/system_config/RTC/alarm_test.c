/*
 * alarm_test.c
 *
 * Created on: Sep 3, 2025
 */
#include "RTC/rtc.h"
#include "print_scan.h"

void do_something1() { printMsg("Do something 1 \n\r"); }
void do_something2() { printMsg("Do something 2 \n\r"); }
void do_something3() { printMsg("Do something 3 \n\r"); }
void do_something4() { printMsg("Do something 4 \n\r"); }
void do_something5() { printMsg("Do something 5 \n\r"); }
void do_something6() { printMsg("Do something 6 \n\r"); }

void testFunction_RTC_Alarm() {
	printMsg("Starting RTC alarm test \n\r");

	// Basic one shot callbacks one after the other
	rtc_scheduleCallback(5, 0, 0, false, do_something1);
	rtc_scheduleCallback(15, 0, 0, false, do_something2);
	rtc_scheduleCallback(30, 0, 0, false, do_something3);
	uint32_t id = rtc_scheduleCallback(35, 0, 0, false, do_something6);

	// Schedule a callback to run in the middle of other tasks
	rtc_scheduleCallback(10, 0, 0, false, do_something4);

	// Schedule a recurring callback
	rtc_scheduleCallback(20, 0, 0, true, do_something5);

	// Check if entry is there
	printMsg("Is entry active: %d\n\r", rtc_isEntryActive(id));
	printMsg("When entry is going to be called: %d\n\r", rtc_getEntry(id).unix_time);

	printMsg("Deleting of entry response: %d\n\r", rtc_deleteEntry(id));

	// Check if entry is there after deleting it
	printMsg("Is entry active: %d\n\r", rtc_isEntryActive(id));
	// Expect 134232923
	printMsg("When entry is going to be called: %d\n\r", rtc_getEntry(id).unix_time);

	// Kill everything
	// Make sure continuous entry stops
	rtc_scheduleCallback(45, 0, 0, false, rtc_deleteAllEntries);

	while (true) {
		continue;
	}
}

