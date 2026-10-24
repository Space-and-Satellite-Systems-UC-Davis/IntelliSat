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

// INSTRUCTIONS FOR TESTING (assuming CubeIDE debugger):
// 1. Flash the PFC
// 2. Use the debugger to check the first few fields of the log
//	  in radio_downlink_idle_log(). boot_counter and year/month
// 3. Flash to Radio with same ID. Set breakpoint on the end of crc_chunked_read()
// 4. Continue until it breaks there.
// 5. Open Intercomm.c and hover over "storedData"
// 6. That should show the contents. Check if bytes match what it was on PFC.
//    Should be some boot count number and then 0, 1, 1. Check step 2.
// 7. If you have the ability to read radio, check it's sending it correctly
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
