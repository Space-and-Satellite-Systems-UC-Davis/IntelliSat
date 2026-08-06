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

    size_t big_len = 20;
    uint8_t chunk[big_len];
    for (size_t i = 0; i < big_len; i++) {
        chunk[i] = i;
    }

    while (true) {
    	radio_downlink_idle_log();
    	delay_ms(2000);

    	// See how memory is overwritten. Needs a solution as of writing.
    	// Ring buffer suggested.
    	radio_push(chunk, sizeof(chunk));
    	delay_ms(2000);

    }
}
