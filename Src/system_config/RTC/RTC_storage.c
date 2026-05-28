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
//	rtc_writeToBKPNumber(0, 0);
	switch (RTC->BKP0R) {
		case 0:
			printMsg("Flag is in Default after-erase state. This should not happen if called after init\n");
			break;
		case 1:
			printMsg("Flag has been initialized to First time state\n");
			break;
		default:
			printMsg("Flag is in Not First state. Boot counter is >1\n");
			break;

	}

	printMsg("\n");
	bool first = rtc_isFirstTime();
	printMsg("First time: %s\n", first ? "true" : "false");

	printMsg("State of previous ADCS Variables: %d\n", RTC->BKP1R);

	// Clear ADCS vars
	rtc_writeToBKPNumber(0, 1);
	printMsg("State of cleared ADCS Variables: %d\n", RTC->BKP1R);

	rtc_writeToADCSVariable(true, Sun0);
	rtc_writeToADCSVariable(true, Sun1);
	rtc_writeToADCSVariable(true, Coil1);
	rtc_writeToADCSVariable(true, Hdd0);

	printMsg("New state ADCS Variables: %d\n", RTC->BKP1R);

	printMsg("State of variable at offset Sun1: %d\n", rtc_readFromADCSVariable(Sun1));
	printMsg("State of variable at offset Hdd0: %d\n", rtc_readFromADCSVariable(Hdd0));
	printMsg("State of variable at offset Hdd1: %d\n", rtc_readFromADCSVariable(Hdd1));

	nop(1);

	/* EXPECTED OUTPUT, ASSUMING ONE CLEARS THE FIRST TIME FLAG AAND THEN RUNS
	Flag has been initialized to First time state

	First time: true
	State of previous ADCS Variables: 532483
	State of cleared ADCS Variables: 0
	New state ADCS Variables: 532483
	State of variable at offset Sun1: 1
	State of variable at offset Hdd0: 1
	State of variable at offset Hdd1: 0
	 */
}
