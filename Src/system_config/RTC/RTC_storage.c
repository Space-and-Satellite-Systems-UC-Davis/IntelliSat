/*
 *  RTC_storage.c
 *
 *  Created on: May 7, 2026
 */

#include "rtc.h"
#include "print_scan.h"

// WARNING: THIS TEST REQUIRES SEVERAL RUNS.
void testFunction_RTC_storage() {
	// Uncomment if you want to clear first flag
	//rtc_writeToBKPNumber(0, 0);
	switch (RTC->BKP0R) {
		case Default:
			printMsg("Flag is in Default after-erase state. This should not happen if called after init\n");
			break;
		case First:
			printMsg("Flag has been initialized to First time state\n");
			break;
		case NotFirst:
			printMsg("Flag is in Not First state\n");
			break;
		default:
	}

	bool first = rtc_isFirstTime();
	printMsg("First time: %s", first ? "true" : "false");

	printMsg("State of previous ADCS Variables: %d", RTC->BKP1R);

	// Clear ADCS vars
	rtc_writeToBKPNumber(0, 1);
	printMsg("State of cleared ADCS Variables: %d", RTC->BKP1R);

	rtc_writeToADCSVariable(true, 0);
	rtc_writeToADCSVariable(true, 1);
	rtc_writeToADCSVariable(true, 31);
	rtc_writeToADCSVariable(true, 30);

	printMsg("New state ADCS Variables: %d", RTC->BKP1R);

	printMsg("State of variable at offset 1: %d", rtc_readFromADCSVariable(1));
	printMsg("State of variable at offset 13: %d", rtc_readFromADCSVariable(13));

	nop(1);
}
