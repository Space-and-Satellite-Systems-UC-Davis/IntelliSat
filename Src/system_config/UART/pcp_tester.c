/******************************************************************************
* File:             pcp_tester.c
*
* Author:           Eric Xu  
* Created:          02/12/25 
* Description:      Tests PCP
*****************************************************************************/
#include <stdint.h>
#include <string.h>

#include <print_scan.h>
#include <Timers/timers.h>
#include <UART/uart.h>
#include <UART/pcp.h>

#define RECV_NBYTES 256

static PCPDevice test_pcpdev;
/** Send log messages. */
static USART_TypeDef* const debug_bus = LPUART1;
/** Send and receive from control_bus */
static USART_TypeDef* const pcpdev_bus = USART2;
// Shared with debug_bus since no debug messages contain packets ('{payload}')
/** Send to and receive from pcpdev_bus */
static USART_TypeDef* const control_bus = LPUART1;

static const uint64_t timeout = 3000;

static void wait(uint64_t time) {
    uint64_t timer = getSysTime();
    while (getSysTime() - timer < time) {}
}

static void flush(USART_TypeDef* bus) {
    uint8_t buf;
    while (usart_receiveBufferNotEmpty(bus))
        usart_receiveBytes(bus, &buf, 1);
}

static bool debug_cmp(char* expected, char* received) {
    if (strcmp(expected, (char*)received) != 0) {
        debugMsg("Failed. Expected:");
        debugMsg(expected);
        debugMsg("Received instead:");
        debugMsg((char*)received);
        return false;
    } else {
        debugMsg("SUCCESS! Received:");
        debugMsg((char*)received);
        return true;
    }
}

static bool debug_retransmission(char* expected) {
    debugMsg("Wait a bit to timeout transmission...");
    wait(timeout);
    debugMsg("Retransmitting...");
    pcp_retransmit(&test_pcpdev);
    uint8_t received[RECV_NBYTES];
    int readnbytes = usart_receiveBytes(control_bus, received, strlen(expected));
    received[readnbytes] = '\0';
    return debug_cmp(expected, (char*)received);
}

static bool debug_control_rx(char* expected) {
    uint8_t received[RECV_NBYTES];
    int readnbytes = usart_receiveBytes(control_bus, received, strlen(expected));
    received[readnbytes] = '\0';
    return debug_cmp(expected, (char*)received);
}

static void test_init() {
    debugMsg("Running PCP Tests");

    debugMsg("Initializing PCP Device bus...");
	usart_init(pcpdev_bus, 9600);
    wait(3000);
    flush(control_bus);
    flush(pcpdev_bus);
    debugMsg("Initializing PCP Device...");
    make_pcpdev_advanced(&test_pcpdev, pcpdev_bus, timeout, 200, 200, 3);
    debugMsg("Setting first sequence number to '0'...");
    test_pcpdev.tx_old_seq = '0';
    test_pcpdev.rx_tail_seq = '0';
}

static void test_tx() {
    debugMsg("[Test transmission] Transmitting three packets...");
    flush(control_bus);
    flush(pcpdev_bus);
    const int payload_len = 9;
    pcp_transmit(&test_pcpdev, (uint8_t*)"Packet 48", payload_len);
    pcp_transmit(&test_pcpdev, (uint8_t*)"Packet 49", payload_len);
    pcp_transmit(&test_pcpdev, (uint8_t*)"Packet 50", payload_len);
    if (!debug_control_rx("{0Packet 48}{1Packet 49}{2Packet 50}"))
        return;

    debugMsg("[Test retransmission]");
    flush(control_bus);
    flush(pcpdev_bus);
    if (!debug_retransmission("{0Packet 48}"))
        return;

    debugMsg("[Test acknowledgement] Acknowledging Packet 48...");
    flush(control_bus);
    flush(pcpdev_bus);
    usart_transmitBytes(control_bus, (uint8_t*)"<0>", 3);
    if (!debug_retransmission("{1Packet 49}"))
        return;

    debugMsg("[Test out of order acknowledgement] Sending Packet 51...");
    flush(control_bus);
    flush(pcpdev_bus);
    pcp_transmit(&test_pcpdev, (uint8_t*)"Packet 51", payload_len);
    debugMsg("Acknowledging Packet 50...");
    flush(control_bus);
    flush(pcpdev_bus);
    usart_transmitBytes(control_bus, (uint8_t*)"<2>", 3);
    if (!debug_retransmission("{1Packet 49}"))
        return;
    flush(control_bus);
    flush(pcpdev_bus);
    debugMsg("Acknowledging Packet 49...");
    usart_transmitBytes(control_bus, (uint8_t*)"<1>", 3);
    if (!debug_retransmission("{3Packet 51}"))
        return;

    debugMsg("[Test invalid acknowledgement] Acknowledging Packet 48");
    flush(control_bus);
    flush(pcpdev_bus);
    usart_transmitBytes(control_bus, (uint8_t*)"<0>", 3);
    if (!debug_retransmission("{3Packet 51}"))
        return;
    debugMsg("Acknowledging Packet 50");
    flush(control_bus);
    flush(pcpdev_bus);
    usart_transmitBytes(control_bus, (uint8_t*)"<2>", 3);
    if (!debug_retransmission("{3Packet 51}"))
        return;
}

static void test_rx() {
    debugMsg("[Test receive] Sending Packet 48");
    flush(control_bus);
    flush(pcpdev_bus);
    usart_transmitBytes(control_bus, (uint8_t*)"{0Packet 48}", 12);
    wait(500);
    pcp_update_rx(&test_pcpdev);
    if (!debug_control_rx("<0>"))
        return;
    char buf[RECV_NBYTES];
    int readnbytes = pcp_receive(&test_pcpdev, (uint8_t*)buf);
    buf[readnbytes] = '\0';
    if (!debug_cmp("Packet 48", buf))
        return;
}

static void playground() {
    //usart_transmitStr(control_bus, (uint8_t*)"Hello, world!");
    //char buf;
    //uint64_t last_pinged = getSysTime();
    //while (true) {
    //    // Ping tx every 3 seconds
    //    if (getSysTime() - last_pinged > 3000) {
    //        usart_transmitStr(control_bus, (uint8_t*)"hi!");
    //        last_pinged = getSysTime();
    //    }
    //    // Echo rx to tx
    //    if (usart_receiveBufferNotEmpty(control_bus)) {
    //        usart_receiveBytes(control_bus, (uint8_t*)&buf, 1);
    //        usart_transmitBytes(control_bus, (uint8_t*)&buf, 1);
    //    }
    //}
    const int payload_len = 9;
    pcp_transmit(&test_pcpdev, (uint8_t*)"Packet 48", payload_len);
    pcp_transmit(&test_pcpdev, (uint8_t*)"Packet 49", payload_len);
    pcp_transmit(&test_pcpdev, (uint8_t*)"Packet 50", payload_len);
    pcp_transmit(&test_pcpdev, (uint8_t*)"Packet 51", payload_len);
    pcp_transmit(&test_pcpdev, (uint8_t*)"Packet 52", payload_len);
}

void testFunction_PCP() {
    test_init();

    //test_tx();
    test_rx();
    //playground();

    debugMsg("PCP Tests complete. Entering interactive mode.");

    const uint64_t interval = 100;
    uint64_t last_update = getSysTime();
    while (true) {
        if (getSysTime() - last_update < interval) {
            continue;
        }
        last_update = getSysTime();
        pcp_retransmit(&test_pcpdev);
    }

	while(1);
}
