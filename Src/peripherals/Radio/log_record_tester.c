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

	delay_ms(3000);
	// While reading stored_data from the other side, you should see:
	// - The log bytes going up to ~200 bytes
	// - Occasionally overwritten by the chunk
    while (true) {
    	// THE ACTUAL "sending log via downlink" PART IS HERE
    	radio_downlink_idle_log();

    	delay_ms(5000);

    	// See how memory is overwritten. Needs a solution as of writing.
    	// Ring buffer suggested.
    	radio_push(chunk, sizeof(chunk));
    }
}
