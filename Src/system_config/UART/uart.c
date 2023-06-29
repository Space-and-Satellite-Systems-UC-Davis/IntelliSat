/*
 * uart.h
 *
 * Change Log:
 *
 *	- May 11, 2023
 *		Author       : Darsh
 *		Log          : Added comments, plus configurable baud rate
 *
 *  - May 6, 2023
 *      Author       : Raphael, Darsh
 *      Contributors : Jacob, Allison, Raymond
 *      Log          : Code for uart3_display, responsible for printing a 'hello!\n' to the console
 *
 */

#include "uart.h"

/*
 * Initializes the UART hardware to conduct UART communication with
 * 1 start bit, 8 data bits, 1 stop bits, No parity
 *
 * @param bus        The USART Bus that needs initialization
 * @param baud_rate  The desired Baud Rate of the UART system
 *
 * @returns None
 */
void uart_init(USART_TypeDef *bus, int baud_rate){
	// Clear control register 1 (disables UART and any random default configurations)
	bus->CR1 = 0;

	// Configurations:
	//   - Word Size to 8 bits (M0:M1 = 00)
	//   - Baud rate to baud_rate
	//   - Stop Bit to 1 bit   (STOP = 00)
	bus->CR1 &= ~(USART_CR1_M0 | USART_CR1_M1);
	bus->BRR = (core_MHz * 1000 * 1000) / baud_rate;	// clk_speed / baud_rate
	bus->CR2 &= ~USART_CR2_STOP;

	// Enable UART3
	bus->CR1 |= USART_CR1_UE;
}

/*
 * Utilizes USART hardware transmitter to send a character
 * to a listerner on the other end (details on pinout in gpio.h)
 *
 * @param bus       The USART Bus doing the tranmission
 * @param message   The character being sent
 *
 * @returns None
 */
void uart_transmitChar(USART_TypeDef *bus, char c) {
	// Enable UART3 and Transmitter
	bus->CR1 |= USART_CR1_UE | USART_CR1_TE;

	// Place the character in the Data Register
	bus->TDR = c;

	// Wait for the Transfer to be completed by monitoring the TC flag
	while(!(bus->ISR & USART_ISR_TC));
}

/*
 * Utilizes USART hardware transmitter to send a variable length string
 * to a listerner on the other end (details on pinout in gpio.h)
 *
 * @param bus       The USART Bus doing the tranmission
 * @param message   The string (character array) being sent
 *
 * @returns None
 */
void uart_transmitStr(USART_TypeDef *bus, char message[]) {
	// Enable UART3 and Transmitter
	bus->CR1 |= USART_CR1_UE | USART_CR1_TE;

	// Transfer each character one at a time
	for (int i = 0; i < strlen(message); i++){
		// wait until Data register is empty
		while (!(bus->ISR & USART_ISR_TXE));
		// Place the character in the Data Register
		bus->TDR = message[i];
	}

	// Wait for the Transfer to be completed by monitoring the TC flag
	while(!(bus->ISR & USART_ISR_TC));
}
