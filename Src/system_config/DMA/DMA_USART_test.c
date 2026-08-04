/*
 * DMA_USART_test.c
 *
 *  Created on: Jul 7, 2026
 */

#include "globals.h"
#include "UART/uart.h"
#include "DMA.h"
#include "print_scan.h"

void testFunction_DMA_USART(void) {
    printMsg("Starting DMA Test...\n");

    const uint32_t timeout_ms = DEFAULT_TIMEOUT_MS;
    const uint8_t message[] = "dma loopback test\n";
    uint8_t rx_buf[sizeof(message)] = {0};

    // Clear stale state
    set_dma_usart_tx(true);
    set_dma_usart_rx(false);

    if (!usart_receiveBytesDMA(SELECT_USART1_RX, rx_buf, sizeof(rx_buf))) {
        printMsg("DMA RX start failed\n");
        return;
    }

    if (!usart_transmitBytesDMA(SELECT_USART1_TX, message, sizeof(message))) {
        printMsg("DMA TX start failed\n");
        return;
    }

    uint64_t start = getSysTime();
    while (!get_dma_usart_tx() && !is_time_out(start, timeout_ms)) {
    }
    if (!get_dma_usart_tx()) {
        printMsg("DMA TX timeout\n");
        return;
    }

    start = getSysTime();
    while (!get_dma_usart_rx() && !is_time_out(start, timeout_ms)) {
    }
    if (!get_dma_usart_rx()) {
        printMsg("DMA RX timeout\n");
        return;
    }

    bool match = true;
    for (uint16_t i = 0; i < sizeof(message); i++) {
        if (rx_buf[i] != message[i]) { match = false; break; }
    }

    if (!match) {
        printMsg("DMA RX mismatch\n");
        return;
    }

    printMsg("DMA loopback OK\n");
}


