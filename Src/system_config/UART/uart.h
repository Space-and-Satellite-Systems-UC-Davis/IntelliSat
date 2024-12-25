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

#include <globals.h>
#include <GPIO/gpio.h>
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
 * Utilizes USART hardware transmitter to send a variable length string
 *
 * @param bus       The USART Bus doing the transmission
 * @param message   The string (character array) being sent
 *
 * @returns None
 */
void usart_transmitBytes(USART_TypeDef *bus, uint8_t message[]);

/*
 * Returns the status of the USART receiver's FIFO buffer
 *
 * @param None
 * @returns Whether the buffer has
 */
bool usart_receiveBufferNotEmpty(USART_TypeDef *bus);

/*
 * Utilizes the USART hardware receiver to get `size` number of bytes from a FIFO buffer.
 * Will wait for more bytes in case enough aren't available.
 * Incorporates a timeout system for safety.
 *
 * @param bus		The USART Bus that will be receiving
 * @param buffer	The buffer where the received bytes must be stored. Length is assumed to be >= `size`
 * @param size		The number of bytes to receive and store
 *
 * @returns			The number of bytes actually received and stored ( <= `size`)
 */
int usart_receiveBytes(USART_TypeDef *bus, uint8_t buffer[], uint16_t size);

#endif	// REALOP1_UART_H_
