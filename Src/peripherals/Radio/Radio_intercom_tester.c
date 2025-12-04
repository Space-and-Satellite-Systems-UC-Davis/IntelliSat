#include "Radio/Radio_intercom.h"
#include "print_scan.h"
#include "globals.h"
#include <string.h>


void testFunction_RadioIntercom() {
    radio_init();
    while (true) {
        uint8_t chunk[CHUNK_LENGTH*4];
        memset(chunk, 42, sizeof chunk);
        printMsg("I'm alive!\r\n");
        printMsg("%d\r\n", radio_push(chunk, sizeof chunk));
        printMsg("%d\r\n", radio_force_pull(chunk).size);
        printMsg("%c\r\n", radio_get_state());
        printMsg("%d\r\n", radio_downlink(chunk, 4));
        printMsg("Cycle Complete\r\n");
    }
}
