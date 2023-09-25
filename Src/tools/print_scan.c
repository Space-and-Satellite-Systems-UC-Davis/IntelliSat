/*
 * print_scan.c
 *
 * 	- May 11, 2023 (Creation)
 * 		Author : Darsh
 * 		Log    : Created the printMsg function
 */


#include "print_scan.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

/*
 * Prints a variable length string to the console. Only prints 128 characters max
 *
 * @param message The string (character array) to be printed
 *
 * @returns None
 */
void printMsg(char *message, ...) {
	// TODO : Add ability to accept variadic strings
	char buff[128];

	va_list args;
	va_start(args, message);
	vsprintf(buff,message,args);

	usart_transmitStr(USART3, buff);
}
