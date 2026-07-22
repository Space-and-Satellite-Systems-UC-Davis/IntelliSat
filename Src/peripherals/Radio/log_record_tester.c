/*
 * log_record_tester.c
 *
 *  Created on: Jul 18, 2026
 */

#include "log_record.h"
#include "print_scan.h"
#include "Radio/Radio_intercom.h"
#include "globals.h"
#include <string.h>

void testFunction_LogDownlink() {
    radio_init();

    uint8_t chunk[200];
    for (int i = 0; i < 200; i++) {
        chunk[i] = i;
    }

    while (true) {
    	log_record_idle log = fill_log_idle();
    	chunk[0] = log.boot_number;
    	bool test1 = radio_push(chunk, sizeof(chunk));

    	// Cast into byte array for sending it
//    	bool test1 = radio_push((uint8_t*)&log, sizeof(log));

    	delay_ms(1000);
//    	bool test2 = radio_downlink(NULL, sizeof(log));
    	printMsg("%d", test1);
//    	printMsg("%d", test2);

    	delay_ms(1000);
    }
}
