/******************************************************************************
* File:             pcp_tester.c
*
* Author:           Eric Xu  
* Created:          02/12/25 
* Description:      Tests PCP
*****************************************************************************/
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <print_scan.h>
#include <Timers/timers.h>
#include <UART/uart.h>
#include <UART/pcp.h>

#define RECV_NBYTES 256

PCPDevice test_pcpdev;
USART_TypeDef* const debug_bus = LPUART1;
USART_TypeDef* const pcpdev_bus = USART1;
/** Send to and receive from pcpdev_bus */
USART_TypeDef* const control_bus = LPUART1;

const uint64_t timeout = 3000;

void debug(char* msg) {
    int now = getSysTime();
    uint8_t str[64];
    sprintf((char*)str, "%8d", now);
    usart_transmitStr(debug_bus, (uint8_t*)"\n");
    usart_transmitStr(debug_bus, str);
    usart_transmitStr(debug_bus, (uint8_t*)":");
    usart_transmitStr(debug_bus, (uint8_t*)msg);
}

bool debug_cmp(char* expected, char* received) {
    if (strcmp(expected, (char*)received) != 0) {
        debug("Failed. Expected:");
        debug(expected);
        debug("Received instead:");
        debug((char*)received);
        return false;
    } else {
        debug("Success! Received:");
        debug((char*)received);
        return true;
    }
}

bool debug_retransmission(char* expected) {
    debug("Wait a bit to timeout transmission...");
    uint64_t timer = getSysTime();
    while (getSysTime() - timer < timeout) {}
    debug("Retransmitting...");
    pcp_retransmit(&test_pcpdev);
    uint8_t received[RECV_NBYTES];
    int readnbytes = usart_receiveBytes(control_bus, received, strlen(expected));
    received[readnbytes] = '\0';
    return debug_cmp(expected, (char*)received);
}

void update() {
    //printMsg("Pulsing%d\n", getSysTime());
    debug("Update loop");
    pcp_retransmit(&test_pcpdev);
}

void testFunction_PCP() {
    debug("Running PCP Tests");

    debug("Initializing PCP Device bus...");
	usart_init(pcpdev_bus, 9600);
    debug("Initializing PCP Device...");
    make_pcpdev_advanced(&test_pcpdev, pcpdev_bus, timeout, 200, 200, 3);
    debug("Setting first sequence number to '0'...");
    test_pcpdev.tx_old_seq = '0';

    debug("[Test transmission] Transmitting three packets...");
    const int payload_len = 9;
    pcp_transmit(&test_pcpdev, (uint8_t*)"Packet 48", payload_len);
    pcp_transmit(&test_pcpdev, (uint8_t*)"Packet 49", payload_len);
    pcp_transmit(&test_pcpdev, (uint8_t*)"Packet 50", payload_len);
    char* expected = "{0Packet 48}{1Packet 49}{2Packet 50}";
    uint8_t received[200];
    int readnbytes = usart_receiveBytes(control_bus, received, strlen(expected));
    received[readnbytes] = '\0';
    debug_cmp(expected, (char*)received);
    debug_retransmission("{0Packet 48}");

    debug("[Test acknowledgement] Acknowledging Packet 48...");
    usart_transmitBytes(control_bus, (uint8_t*)"<0>", 3);
    debug_retransmission("{1Packet 49}");

    debug("[Test out of order acknowledgement] Sending Packet 51...");
    pcp_transmit(&test_pcpdev, (uint8_t*)"Packet 51", payload_len);
    debug("Acknowledging Packet 50...");
    usart_transmitBytes(control_bus, (uint8_t*)"<2>", 3);
    debug_retransmission("{1Packet 49}");
    debug("Acknowledging Packet 49...");
    usart_transmitBytes(control_bus, (uint8_t*)"<1>", 3);
    debug_retransmission("{3Packet 51}");

    debug("[Test invalid acknowledgement] Acknowledging Packet 48");
    usart_transmitBytes(control_bus, (uint8_t*)"<0>", 3);
    debug_retransmission("{3Packet 51}");
    debug("Acknowledging Packet 50");
    usart_transmitBytes(control_bus, (uint8_t*)"<2>", 3);
    debug_retransmission("{3Packet 51}");

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

