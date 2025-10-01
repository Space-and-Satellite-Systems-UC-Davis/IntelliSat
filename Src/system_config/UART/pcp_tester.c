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
/** Send and receive from control_bus */
static USART_TypeDef* const pcpdev_bus = USART2;
/** Send to and receive from pcpdev_bus */
static USART_TypeDef* const control_bus = LPUART1;

/** Time between transmission and retransmission */
static const uint64_t pcpdev_timeout = 3000;
/** Maximum time between transmission and reception */
static const uint64_t tx_timeout = 200;

static void wait(uint64_t time) {
    uint64_t timer = getSysTime();
    while (getSysTime() - timer < time) {}
}

static bool debug_cmp(char* expected, char* actual) {
    if (strcmp(expected, (char*)actual) != 0) {
        debugMsg("Failed. Expected message of length %d:", strlen(expected));
        debugMsg(expected);
        debugMsg("Received instead message of length %d:", strlen(actual));
        debugMsg((char*)actual);
        return false;
    } else {
        debugMsg("SUCCESS! Received:");
        debugMsg((char*)actual);
        return true;
    }
}

static bool debug_retransmission(char* expected) {
    debugMsg("Wait a bit to timeout transmission...");
    wait(pcpdev_timeout);
    debugMsg("Retransmitting...");
    pcp_retransmit(&test_pcpdev);
    uint8_t received[RECV_NBYTES];
    int readnbytes = usart_receiveBytes(control_bus, received, strlen(expected));
    received[readnbytes] = '\0';
    return debug_cmp(expected, (char*)received);
}

static bool debug_control_rx(char* expected) {
    uint8_t received[RECV_NBYTES];
    int readnbytes = 0;
    while (usart_receiveBufferNotEmpty(control_bus))
        readnbytes += usart_receiveBytes(control_bus, received, strlen(expected));
    received[readnbytes] = '\0';
    return debug_cmp(expected, (char*)received);
}

static void test_init() {
    debugMsg("Running PCP Tests");

    debugMsg("Initializing PCP Device bus...");
	usart_init(pcpdev_bus, 9600);
    wait(tx_timeout);
    usart_flushrx(control_bus);
    usart_flushrx(pcpdev_bus);
    debugMsg("Initializing PCP Device...");
    make_pcpdev_advanced(&test_pcpdev, pcpdev_bus, pcpdev_timeout, 200, 200, 3);
    debugMsg("Setting first sequence number to '0'...");
    test_pcpdev.tx_old_seq = '0';
    test_pcpdev.rx_tail_seq = '0';
}

static void test_tx() {
    debugMsg("[Test transmission] Transmitting three packets...");
    usart_flushrx(control_bus);
    usart_flushrx(pcpdev_bus);
    const int payload_len = 9;
    pcp_transmit(&test_pcpdev, (uint8_t*)"Packet 48", payload_len);
    pcp_transmit(&test_pcpdev, (uint8_t*)"Packet 49", payload_len);
    pcp_transmit(&test_pcpdev, (uint8_t*)"Packet 50", payload_len);
    if (!debug_control_rx("{0Packet 48}{1Packet 49}{2Packet 50}"))
        return;

    debugMsg("[Test retransmission]");
    usart_flushrx(control_bus);
    usart_flushrx(pcpdev_bus);
    if (!debug_retransmission("{0Packet 48}"))
        return;

    debugMsg("[Test acknowledgement] Acknowledging Packet 48...");
    usart_flushrx(control_bus);
    usart_flushrx(pcpdev_bus);
    usart_transmitBytes(control_bus, (uint8_t*)"<0>", 3);
    if (!debug_retransmission("{1Packet 49}"))
        return;

    debugMsg("[Test out of order acknowledgement] Sending Packet 51...");
    usart_flushrx(control_bus);
    usart_flushrx(pcpdev_bus);
    pcp_transmit(&test_pcpdev, (uint8_t*)"Packet 51", payload_len);
    debugMsg("Acknowledging Packet 50...");
    usart_flushrx(control_bus);
    usart_flushrx(pcpdev_bus);
    usart_transmitBytes(control_bus, (uint8_t*)"<2>", 3);
    if (!debug_retransmission("{1Packet 49}"))
        return;
    usart_flushrx(control_bus);
    usart_flushrx(pcpdev_bus);
    debugMsg("Acknowledging Packet 49...");
    usart_transmitBytes(control_bus, (uint8_t*)"<1>", 3);
    if (!debug_retransmission("{3Packet 51}"))
        return;

    debugMsg("[Test invalid acknowledgement] Acknowledging Packet 48");
    usart_flushrx(control_bus);
    usart_flushrx(pcpdev_bus);
    usart_transmitBytes(control_bus, (uint8_t*)"<0>", 3);
    if (!debug_retransmission("{3Packet 51}"))
        return;
    debugMsg("Acknowledging Packet 50");
    usart_flushrx(control_bus);
    usart_flushrx(pcpdev_bus);
    usart_transmitBytes(control_bus, (uint8_t*)"<2>", 3);
    if (!debug_retransmission("{3Packet 51}"))
        return;
}

static void test_rx() {
    debugMsg("[Test receive] Sending Packet 48...");
    usart_flushrx(control_bus);
    usart_flushrx(pcpdev_bus);
    usart_transmitBytes(control_bus, (uint8_t*)"{0Packet 48}", 12);
    wait(tx_timeout);
    pcp_update_rx(&test_pcpdev);
    if (!debug_control_rx("<0>"))
        return;
    char buf[RECV_NBYTES];
    int readnbytes = pcp_read(&test_pcpdev, (uint8_t*)buf);
    buf[readnbytes] = '\0';
    if (!debug_cmp("Packet 48", buf))
        return;

    debugMsg("[Test multi-receive] Sending Packet 49, 50, and 51...");
    usart_flushrx(control_bus);
    usart_flushrx(pcpdev_bus);
    usart_transmitBytes(control_bus, (uint8_t*)"{1Packet 49}", 12);
    usart_transmitBytes(control_bus, (uint8_t*)"{2Packet 50}", 12);
    usart_transmitBytes(control_bus, (uint8_t*)"{3Packet 51}", 12);
    wait(tx_timeout);
    readnbytes = pcp_read(&test_pcpdev, (uint8_t*)buf);
    buf[readnbytes] = '\0';
    if (!debug_cmp("Packet 49", buf))
        return;
    readnbytes = pcp_read(&test_pcpdev, (uint8_t*)buf);
    buf[readnbytes] = '\0';
    if (!debug_cmp("Packet 50", buf))
        return;
    readnbytes = pcp_read(&test_pcpdev, (uint8_t*)buf);
    buf[readnbytes] = '\0';
    if (!debug_cmp("Packet 51", buf))
        return;
    readnbytes = pcp_read(&test_pcpdev, (uint8_t*)buf);
    buf[readnbytes] = '\0';
    if (readnbytes != -1) {
        debugMsg("Failed. Shouldn't have any extra packets.");
        debugMsg("Read message of length %d: %s", readnbytes, buf);
        return;
    }
    if (!debug_control_rx("<1><2><3>"))
        return;

    debugMsg("[Test out-of-order send] Sending Packet 54, 53, and 52...");
    usart_flushrx(control_bus);
    usart_flushrx(pcpdev_bus);
    usart_transmitBytes(control_bus, (uint8_t*)"{6Packet 54}", 12);
    wait(tx_timeout);
    pcp_update_rx(&test_pcpdev);
    if (!debug_control_rx("<3>"))
        return;
    usart_transmitBytes(control_bus, (uint8_t*)"{5Packet 53}", 12);
    wait(tx_timeout);
    pcp_update_rx(&test_pcpdev);
    if (!debug_control_rx("<3>"))
        return;
    usart_transmitBytes(control_bus, (uint8_t*)"{4Packet 52}", 12);
    wait(tx_timeout);
    pcp_update_rx(&test_pcpdev);
    if (!debug_control_rx("<6>"))
        return;
    readnbytes = pcp_read(&test_pcpdev, (uint8_t*)buf);
    buf[readnbytes] = '\0';
    if (!debug_cmp("Packet 52", buf))
        return;
    readnbytes = pcp_read(&test_pcpdev, (uint8_t*)buf);
    buf[readnbytes] = '\0';
    if (!debug_cmp("Packet 53", buf))
        return;
    readnbytes = pcp_read(&test_pcpdev, (uint8_t*)buf);
    buf[readnbytes] = '\0';
    if (!debug_cmp("Packet 54", buf))
        return;
    readnbytes = pcp_read(&test_pcpdev, (uint8_t*)buf);
    buf[readnbytes] = '\0';
    if (readnbytes != -1) {
        debugMsg("Failed. Shouldn't have any extra packets.");
        debugMsg("Read message of length %d: %s", readnbytes, buf);
        return;
    }
}

void testFunction_PCP() {
    test_init();

    test_tx();
    test_rx();

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
