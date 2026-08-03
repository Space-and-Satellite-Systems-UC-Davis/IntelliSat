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
    size_t small_len = CHUNK_LENGTH;

    uint8_t chunk[big_len];
    for (size_t i = 0; i < big_len; i++) {
        chunk[i] = i;
    }
    uint8_t small_chunk[small_len];
    for (size_t i = 0; i < small_len; i++) {
        small_chunk[i] = i+big_len;
    }

	delay_ms(3000);
    while (true) {
    	radio_push(chunk, sizeof(chunk));
    	delay_ms(2000);
    	// See how memory is overwritten. Needs a solution as of writing.
    	// Ring buffer suggested.
    	radio_push(small_chunk, sizeof(small_chunk));

    	delay_ms(5000);



    	// THE ACTUAL "sending log via downlink" PART IS HERE
    	log_record_idle log = fill_log_idle();
    	uint8_t* log_bytes = (uint8_t*) &log;
		radio_push(log_bytes, sizeof(log));
		delay_ms(2000);

		// radio_push to stuff what we need into memory. Downlink later.
    	radio_downlink(NULL, sizeof(log));
    	delay_ms(2000);
    }
}
