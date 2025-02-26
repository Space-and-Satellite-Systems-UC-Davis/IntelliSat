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

static int read(USART_TypeDef* bus, uint8_t* buf) {
    int len = 0;
    while (usart_receiveBufferNotEmpty(bus)) {
        usart_receiveBytes(bus, (uint8_t*)buf + len, 1);
        len++;
    }
    buf[len] = '\0';
    return len;
}

static bool run_test() {
    // Buffers for bus1 and bus2 respectively
    uint8_t buf1[50];
    int buf1_len = 0;
    uint8_t buf2[50];
    int buf2_len = 0;

    debugMsg("Beginning test");
    debugMsg("[bus1 transmit, bus2 receive]");

    char *test_msg = "Hello, world!";
    debugMsg("Transmitting \"%s\"...", test_msg);
    wait(200);
    usart_flushrx(bus1);
    usart_flushrx(bus2);
    usart_transmitBytes(bus1, (uint8_t*)test_msg, strlen(test_msg));
    wait(200);
    buf1_len = read(bus1, buf1);
    buf1[buf1_len] = '\0';
    buf2_len = read(bus2, buf2);
    buf2[buf2_len] = '\0';
    debugMsg("Checking buf1 ...");
    if (!debug_cmp("", buf1)) return false;
    debugMsg("Checking buf2...");
    if (!debug_cmp(test_msg, buf2)) return false;

    debugMsg("Test completed!");
    return true;
}

void testFunction_UART() {
    init();
    run_test();
}

