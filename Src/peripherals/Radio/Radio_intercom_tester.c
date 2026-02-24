#include "Radio/Radio_intercom.h"
#include "print_scan.h"
#include "globals.h"
#include <string.h>


void testFunction_RadioIntercom() {
    radio_init();
    while (true) {
        uint8_t chunk[CHUNK_LENGTH*4];
        for (int i = 0; i < CHUNK_LENGTH*4; i++) {
            chunk[i] = i + '0';
        }
        printMsg("Chunk is: '%s'\r\n", chunk);
        printMsg("I'm alive!\r\n");
        printMsg("%d\r\n", radio_push(chunk, sizeof chunk));
        printMsg("%d\r\n", radio_force_pull(chunk).size);
        printMsg("Chunk is: '%s'\r\n", chunk);
        printMsg("%c\r\n", radio_get_state());
        printMsg("%d\r\n", radio_downlink(chunk, 4));
        printMsg("Cycle Complete\r\n");
    }
}
