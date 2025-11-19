#include "Radio/Radio_intercom.h"
#include "print_scan.h"
#include "globals.h"
#include <string.h>


void testFunction_RadioIntercom() {
    radio_init();
    while (1) {
        uint8_t chunk[CHUNK_LENGTH*4];
        for (int i = 0; i < CHUNK_LENGTH*4; i++) {
            chunk[i] = 42;
        }
        delay_ms(100);
        printMsg("%d\r\n", radio_push(chunk, sizeof chunk));
        delay_ms(100);
        printMsg("%d\r\n", radio_force_pull(chunk).size);
        delay_ms(100);
        printMsg("%c\r\n", radio_get_state());
        delay_ms(100);
        printMsg("%d\r\n", radio_downlink(chunk, 4));
        delay_ms(100);
        printMsg("Cycle Complete\r\n");
    }
}
