// 2/23/2024

/*
 * This file shows how the UART system interface is supposed to be used
 * In order to follow along, read the following section carefully
 *
 * This document wraps the actual usage examples into smaller functions for
 * sake of readability. The functions defined here don't need to be used as is
 */


// To get the UART system, first include it's header file
//#include <UART/uart.h>
#include "uart.h"

// Before making use of UART, it must be initialized
void InitUart() {
	// this following function will configure the UART1 hardware peripheral
	// to work at 9600 bits per second.
	usart_init(USART1, 9600);
	// There can be multiple UART peripherals, such as USART1, USART2, LPUART1, etc...
}


// There are 2 things that can be done with UART: Transmission and Reception

// Transmitter
void Transmit() {
	// this will contain the data that we want to transmit
	uint8_t buffer[10] = {'d', 'a', 't', 'a'};

	// this will transmit the first 3 bytes from `buffer` over USART1
	usart_transmitBytes(USART1, buffer, 3);
}


// Receiver
void Recieve() {
	// this will be where the data is received
	uint8_t buffer[5] = {' '};

	// this will receive 5 bytes from USART1 into the buffer
	int got = usart_recieveBytes(USART1, buffer, 5);

	// NOTE: `got` tells you how many bytes were actually received.
	// Example: if got = 3, buffer = {'c', 'c', 'c', ' ', ' '}
	//								   ^ New Data     ^ Old Data
}

// Sometimes the Receiver has no data, and so `got` would be 0.
// Alternatively, you can use the following function to check if
// the receiver has data:
void Check() {
	if (usart_recieveBufferNotEmpty()) {
		// receive
	} else {
		// do something else as there is no data to receive
	}
}

