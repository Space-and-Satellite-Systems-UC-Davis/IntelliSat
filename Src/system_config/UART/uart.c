/*
 * uart.h
 *
 * 	- September 22, 2023
 * 		Author	: Darsh
 * 		Log		: Added Clocks and GPIO
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

/************************ GPIO INITIALIZATION HELPERS ************************/

void usart1_gpio_init();
void usart2_gpio_init();

void usart3_gpio_init() {
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;
	while (GPIOC->OTYPER == 0xFFFFFFFF);

	// configure the USART3 Pins to Alternate Function mode
	GPIOC->MODER &= ~(GPIO_MODER_MODE4_Msk | GPIO_MODER_MODE5_Msk);
	GPIOC->MODER |= (GPIO_MODER_MODE4_1 | GPIO_MODER_MODE5_1);

	// configure each pin to AF7
	GPIOC->AFR[0] &= ~(GPIO_AFRL_AFSEL4_Msk | GPIO_AFRL_AFSEL5_Msk);
	GPIOC->AFR[0] |= (7U << GPIO_AFRL_AFSEL4_Pos) | (7U << GPIO_AFRL_AFSEL5_Pos);
}

void uart4_gpio_init();
void uart5_gpio_init();


/*************************** USART INITIALIZATIONS ***************************/

/*
 * Initializes the UART hardware to conduct UART communication with
 * 1 start bit, 8 data bits, 1 stop bits, No parity
 *
 * @param bus        The USART Bus that needs initialization
 * @param baud_rate  The desired Baud Rate of the UART system
 *
 * @returns None
 */
bool usart_init(USART_TypeDef *bus, int baud_rate){
	switch((int)bus) {
		case (int)USART1:
			RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
			break;
		case (int)USART2:
			RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;
			break;
		case (int)USART3:
			RCC->APB1ENR1 |= RCC_APB1ENR1_USART3EN;
			usart3_gpio_init();
			break;
		case (int)UART4:
			RCC->APB1ENR1 |= RCC_APB1ENR1_UART4EN;
			break;
		case (int)UART5:
			RCC->APB1ENR1 |= RCC_APB1ENR1_UART5EN;
			break;
		case (int)LPUART1:
			RCC->APB1ENR2 |= RCC_APB1ENR2_LPUART1EN;
			break;
		default:
			return false;
	}

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

/**************************** USART COMMUNICATION ****************************/

/*
 * Utilizes USART hardware transmitter to send a character
 * to a listerner on the other end (details on pinout in gpio.h)
 *
 * @param bus       The USART Bus doing the tranmission
 * @param message   The character being sent
 *
 * @returns None
 */
void usart_transmitChar(USART_TypeDef *bus, char c) {
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
void usart_transmitStr(USART_TypeDef *bus, char message[]) {
	// Enable UART3 and Transmitter
	bus->CR1 |= USART_CR1_UE | USART_CR1_TE;

	// Transfer each character one at a time
	for (int i = 0; i < (int)strlen(message); i++){
		// wait until Data register is empty
		while (!(bus->ISR & USART_ISR_TXE));
		// Place the character in the Data Register
		bus->TDR = message[i];
	}

	// Wait for the Transfer to be completed by monitoring the TC flag
	while(!(bus->ISR & USART_ISR_TC));
}

