/*
 * print_scan.c
 *
 * 	- May 11, 2023 (Creation)
 * 		Author : Darsh
 * 		Log    : Created the printMsg function
 */


#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "print_scan.h"
#include <UART/uart.h>

#if OP_REV == 1
#define ConsoleUART 	USART3
#elif OP_REV == 2 || OP_REV == 3
#define ConsoleUART 	LPUART1
#endif

#define UART_BAUDRATE	9600

void printer_init() {
	usart_init(ConsoleUART, UART_BAUDRATE);
}

int printMsg(const char *message, ...) {
	char buff[128];

	va_list args;
	va_start(args, message);
	int len = vsnprintf(buff, sizeof(buff), message, args);	// longer messages get cut off instead of overflowing buff
	va_end(args);

	usart_transmitStr(ConsoleUART, (uint8_t *)buff);
	return len;
}

bool scanChar(char *c) {
	return usart_receiveBytes(ConsoleUART, (uint8_t *)c, 1) == 1;
}
