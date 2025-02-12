/******************************************************************************
* File:             pcp_tester.c
*
* Author:           Eric Xu  
* Created:          02/12/25 
* Description:      Tests PCP
*****************************************************************************/
#include <stdint.h>
#include <stdio.h>

#include <print_scan.h>
#include <Timers/timers.h>
#include <UART/uart.h>

void debug(char* msg) {
    uint64_t now = getSysTime();
    char str[64];
    sprintf(str, "%8d", now);
    usart_transmitStr(LPUART1, str);
    usart_transmitStr(LPUART1, ":");
    usart_transmitStr(LPUART1, msg);
    usart_transmitStr(LPUART1, "\n");
}

void update() {
    //printMsg("Pulsing%d\n", getSysTime());
    debug("");
}

void testFunction_PCP() {
    debug("Running PCP Tests");

    const uint64_t interval = 1000;
    uint64_t last_update = getSysTime();
    while (true) {
        if (getSysTime() - last_update > interval) {
            update();
            last_update = getSysTime();
        }
    }

    debug("PCP Tests complete");
	while(1);
}

