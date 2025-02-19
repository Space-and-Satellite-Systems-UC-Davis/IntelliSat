/******************************************************************************
* File:             pcp_tester.c
*
* Author:           Eric Xu  
* Created:          02/12/25 
* Description:      Tests PCP
*****************************************************************************/
#include <print_scan.h>
#include <Timers/timers.h>
#include <UART/uart.h>
#include <UART/pcp.h>

static USART_TypeDef* const bus1 = USART2;
static USART_TypeDef* const bus2 = LPUART1;
static int baudrate = 9600;

static void wait(uint64_t time) {
    uint64_t timer = getSysTime();
    while (getSysTime() - timer < time) {}
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

/**
 * Initialize testing environment
 */
static void init() {
    usart_init(bus1, baudrate);
    usart_init(bus2, baudrate);
}

static bool run_test() {
    // Buffers for bus1 and bus2 respectively
    char buf1[50];
    int buf1_len = 0;
    char buf2[50];
    int buf2_len = 0;

    debugMsg("Beginning test");
    debugMsg("[bus1 transmit, bus2 receive]");
    usart_flushrx(bus1);
    usart_flushrx(bus2);

    strcpy(buf1, "Hello, world!");
    buf1_len = strlen(buf1);
    usart_transmitBytes(bus1, (uint8_t*)buf1, buf1_len);
    wait(500);
    // Check buf2 RX
    while (usart_receiveBufferNotEmpty(bus2)) {
        usart_receiveBytes(bus2, (uint8_t*)buf2 + buf2_len, 1);
        buf2_len++;
    }
    buf2[buf2_len] = '\0';
    if (strcmp(buf1, buf2) != 0) {
        debugMsg("Buffer 2 RX is unexpected. See following message.");
        debug_cmp(buf1, buf2);
        return false;
    }
    // Check buf1 RX
    buf1_len = 0;
    while (usart_receiveBufferNotEmpty(bus1)) {
        usart_receiveBytes(bus1, (uint8_t*)buf1 + buf1_len, 1);
        buf1_len++;
    }
    buf1[buf1_len] = '\0';
    if (strcmp(buf1, "") != 0) {
        debugMsg("Buffer 1 RX is unexpected. See following message.");
        debug_cmp("", buf1);
        return false;
    }
    return true;
}

void testFunction_UART() {
    debugMsg("Testing UART");
    init();
    run_test();
}

