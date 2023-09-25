/*
 * uart.h
 *
 *	- May 11, 2023
 *		Author       : Darsh
 *		Log          : Added comments, plus configurable baud rate
 *
 *  - May 6, 2023
 *      Author       : Raphael, Darsh
 *      Contributors : Jacob, Allison, Raymond
 *      Log          : Initial Setup
 *
 */

#ifndef REALOP1_UART_H_
#define REALOP1_UART_H_

#include "../../globals.h"
#include "../GPIO/gpio.h"
#include <string.h>

/*
 * Initializes the UART hardware to conduct UART communication with
 * 1 start bit, 8 data bits, 1 stop bits, No parity
 *
 * @param bus        The USART Bus that needs initialization
 * @param baud_rate  The desired Baud Rate of the UART system
 *
 * @returns None
 */
bool usart_init(USART_TypeDef *bus, int baud_rate);

/*
 * Utilizes USART hardware transmitter to send a character
 * to a listerner on the other end (details on pinout in gpio.h)
 *
 * @param bus       The USART Bus doing the tranmission
 * @param message   The character being sent
 *
 * @returns None
 */
void usart_transmitChar(USART_TypeDef *bus, char c);

/*
 * Utilizes USART hardware transmitter to send a variable length string
 * to a listerner on the other end (details on pinout in gpio.h)
 *
 * @param bus       The USART Bus doing the tranmission
 * @param message   The string (character array) being sent
 *
 * @returns None
 */
void usart_transmitStr(USART_TypeDef *bus, char message[]);

#endif	// REALOP1_UART_H_
