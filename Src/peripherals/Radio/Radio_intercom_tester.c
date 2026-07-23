#include "Radio/Radio_intercom.h"
#include "print_scan.h"
#include "globals.h"
#include <string.h>


void testFunction_RadioIntercom() {
    radio_init();
    while(true){
        char str[200] = "Hello World! I hope you are receiving this message right now :) I just wanted to test if like 200 characters worked through the intercom. Hope you have a lot of fun debugging everything radio! yippee!";
        printMsg("SENDING MSG!\r\n");
        radio_push(str, 200);
        printMsg("State: %d\r\n", radio_get_state());
        delay_ms(1000);
    }
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
        printMsg("Chunk is: '%s'\r\n", chunk);
        printMsg("Cycle Complete\r\n");
    }
}
