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

/** IMPORTANT: Dealing with Revision Changes
 * Things to change across revisions:
 * 		- USART HELPERS
 * 			- Ensure the correct USART_ReceiverBuffer are declared
 * 			  based on the USART hardwares in use for that revision
 * 			- Ensure the uart_revisionBusDistinguisher() function is
* 			  inline with the USART hardwares in use for that revision
* 		- GPIO HELPERS
* 			- For each gpio_init(), ensure the right pins are
* 			  initialized, depending on the pinout for that revision
*		- USART INTERRUPTS
*			- Ensure the correct USART Hardwares have their interrupts
*			  defined
 */

/************************ USART HELPERS ************************/

#define ReceiveBufferLen 128
struct _rx_fifo {
	uint8_t buffer[ReceiveBufferLen];
	uint16_t front;
	uint16_t rear;
	bool timedout;
};
typedef struct _rx_fifo USART_ReceiverBuffer;

#if OP_REV == 1

USART_ReceiverBuffer USART1_RxBuffer;
USART_ReceiverBuffer USART3_RxBuffer;

#elif OP_REV == 2 || OP_REV == 3

USART_ReceiverBuffer USART1_RxBuffer;
USART_ReceiverBuffer LPUART1_RxBuffer;

#endif


USART_ReceiverBuffer* uart_revisionBusDistinguisher(USART_TypeDef *bus) {
	USART_ReceiverBuffer *rxbuff = NULL;

#if OP_REV == 1
	if (bus == USART1) {
		rxbuff = &USART1_RxBuffer;
	} else if (bus == USART3) {
		rxbuff = &USART3_RxBuffer;
	}
#elif OP_REV == 2 || OP_REV == 3
	if (bus == USART1) {
		rxbuff = &USART1_RxBuffer;
	} else if (bus == LPUART1) {
		rxbuff = &LPUART1_RxBuffer;
	}
#endif

	return rxbuff;
}

#define enqueueBuffer(buff,usart) buff.buffer[buff.rear] = usart->RDR; \
							      buff.rear = (buff.rear + 1) % ReceiveBufferLen;

/************************ GPIO INITIALIZATION HELPERS ************************/

void usart1_gpio_init() {

	/*
	 * OP REV 2 GPIO
	 * 		TX		GPIO B 6		Alternate Function 7
	 * 		RX		GPIO B 7		Alternate Function 7
	 * 		RTS		// MUST BE FORCED - HIGH
	 * 		CTS		// MUST BE FORCED - HIGH
	 *
	 * OP REV 1 GPIO
	 * 		TX		GPIO A 9		Alternate Function 7
	 * 		RX		GPIO G 10		Alternate Function 7
	 * 		RTS		// MUST BE FORCED - HIGH
	 * 		CTS		// MUST BE FORCED - HIGH
	 */

	//	TODO: Forcing RTS and CTS High

#if OP_REV == 1

	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOGEN;
	wait_with_timeout(is_GPIOA_not_ready, DEFAULT_TIMEOUT_MS);
	wait_with_timeout(is_GPIOG_not_ready, DEFAULT_TIMEOUT_MS);

	// configure the USART Pins to Alternate Function mode
	GPIOA->MODER &= ~(GPIO_MODER_MODE9_Msk);
	GPIOG->MODER &= ~(GPIO_MODER_MODE10_Msk);

	GPIOA->MODER |= GPIO_MODER_MODE9_1;
	GPIOG->MODER |= GPIO_MODER_MODE10_1;

	// configure each pin to AF7
	GPIOA->AFR[1] &= ~(GPIO_AFRH_AFSEL9_Msk);
	GPIOG->AFR[1] &= ~(GPIO_AFRH_AFSEL10_Msk);

	GPIOA->AFR[1] |= (7U << GPIO_AFRH_AFSEL9_Msk);
	GPIOG->AFR[1] |= (7U << GPIO_AFRH_AFSEL10_Msk);

#elif OP_REV == 2

	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
	wait_with_timeout(is_GPIOB_not_ready, DEFAULT_TIMEOUT_MS);

	// configure the USART Pins to Alternate Function mode
	GPIOB->MODER &= ~(GPIO_MODER_MODE6_Msk | GPIO_MODER_MODE7_Msk);
	GPIOB->MODER |= (GPIO_MODER_MODE6_1 | GPIO_MODER_MODE7_1);

	// configure each pin to AF7
	GPIOB->AFR[0] &= ~(GPIO_AFRL_AFSEL6_Msk | GPIO_AFRL_AFSEL7_Msk);
	GPIOB->AFR[0] |= (7U << GPIO_AFRL_AFSEL6_Pos) | (7U << GPIO_AFRL_AFSEL7_Pos);
#elif OP_REV == 3 
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOGEN;
	wait_with_timeout(is_GPIOG_not_ready, DEFAULT_TIMEOUT_MS);


	// configure the USART Pins to Alternate Function mode
	GPIOG->MODER &= ~(GPIO_MODER_MODE9_Msk | GPIO_MODER_MODE10_Msk);
	GPIOG->MODER |= (GPIO_MODER_MODE9_1 | GPIO_MODER_MODE10_1);

	// configure each pin to AF7
	GPIOG->AFR[1] &= ~(GPIO_AFRH_AFSEL9_Msk | GPIO_AFRH_AFSEL10_Msk);
	GPIOG->AFR[1] |= (GPIO_AFRX_AF7 << GPIO_AFRH_AFSEL9_Pos) | (GPIO_AFRX_AF7 << GPIO_AFRH_AFSEL10_Pos);
#endif

	return;

}
void usart2_gpio_init() {
#if OP_REV == 3
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIODEN;
	wait_with_timeout(is_GPIOD_not_ready, DEFAULT_TIMEOUT_MS);

	GPIOD->MODER &= ~(GPIO_MODER_MODE5_Msk | GPIO_MODER_MODE6_Msk);
	GPIOD->MODER |= (GPIO_MODER_MODE5_1 | GPIO_MODER_MODE6_1);

	// configure each pin to AF7
	GPIOD->AFR[0] &= ~(GPIO_AFRL_AFSEL5_Msk | GPIO_AFRL_AFSEL6_Msk);
	GPIOD->AFR[0] |= (GPIO_AFRX_AF7 << GPIO_AFRL_AFSEL6_Pos) | (GPIO_AFRX_AF7 << GPIO_AFRL_AFSEL5_Pos);
#endif
	return;

}

void usart3_gpio_init() {

#if OP_REV == 1

	/*
	 * OP REV 1 GPIO
	 * 		TX		GPIO C 4		Alternate Function 7
	 * 		RX		GPIO C 5		Alternate Function 7
	 */

	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;
	wait_with_timeout(is_GPIOC_not_ready, DEFAULT_TIMEOUT_MS);

	// configure the USART Pins to Alternate Function mode
	GPIOC->MODER &= ~(GPIO_MODER_MODE4_Msk | GPIO_MODER_MODE5_Msk);
	GPIOC->MODER |= (GPIO_MODER_MODE4_1 | GPIO_MODER_MODE5_1);

	// configure each pin to AF7
	GPIOC->AFR[0] &= ~(GPIO_AFRL_AFSEL4_Msk | GPIO_AFRL_AFSEL5_Msk);
	GPIOC->AFR[0] |= (7U << GPIO_AFRL_AFSEL4_Pos) | (7U << GPIO_AFRL_AFSEL5_Pos);

#endif

	return;
}

void uart4_gpio_init() {

	return;
}
void uart5_gpio_init() {

	return;
}

void lpuart_gpio_init() {

#if OP_REV == 2

	/*
	 * OP REV 2 GPIO
	 * 		TX		GPIO C 1		Alternate Function 8
	 * 		RX		GPIO C 0		Alternate Function 8
	 */

	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;
	wait_with_timeout(is_GPIOC_not_ready, DEFAULT_TIMEOUT_MS);

	// configure the LPUART Pins to Alternate Function mode
	GPIOC->MODER &= ~(GPIO_MODER_MODE0_Msk | GPIO_MODER_MODE1_Msk);
	GPIOC->MODER |= (GPIO_MODER_MODE0_1 | GPIO_MODER_MODE1_1);

	// configure each pin to AF7
	GPIOC->AFR[0] &= ~(GPIO_AFRL_AFSEL0_Msk | GPIO_AFRL_AFSEL1_Msk);
	GPIOC->AFR[0] |= (8U << GPIO_AFRL_AFSEL0_Pos) | (8U << GPIO_AFRL_AFSEL1_Pos);

#elif OP_REV == 3
	/*
	 * OP REV 2 GPIO
	 * 		TX		GPIO G 7		Alternate Function 8
	 * 		RX		GPIO G 8		Alternate Function 8
	 */

	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOGEN;
	wait_with_timeout(is_GPIOG_not_ready, DEFAULT_TIMEOUT_MS);

	// configure the LPUART Pins to Alternate Function mode
	GPIOG->MODER &= ~(GPIO_MODER_MODE7_Msk | GPIO_MODER_MODE8_Msk);
	GPIOG->MODER |= (GPIO_MODER_MODE7_1 | GPIO_MODER_MODE8_1);

	// configure each pin to AF8
	GPIOG->AFR[0] &= ~(GPIO_AFRL_AFSEL7_Msk);
	GPIOG->AFR[1] &= ~(GPIO_AFRH_AFSEL8_Msk);
	GPIOG->AFR[0] |= (GPIO_AFRX_AF8 << GPIO_AFRL_AFSEL7_Pos);
	GPIOG->AFR[1] |= (GPIO_AFRX_AF8 << GPIO_AFRH_AFSEL8_Pos);

#endif

	return;
}


/*************************** USART INITIALIZATIONS ***************************/

void uart_8bit_1stop(USART_TypeDef *bus, int baud_rate, bool rts_cts_control) {
	// all UARTs use HSI16 as per core_config.c,init_core_clocks()
	uint32_t uart_clock_speed = 16000000;
	if (bus == LPUART1) {
		// prep for baud rate calculation
		uart_clock_speed *= 256;
	}

	// disables UART and any random default configurations
	bus->CR1 = 0;

	// Configurations:
	//   - Word Size to 8 bits (M0:M1 = 00)
	//   - Baud rate to `baud_rate`
	//   - Stop Bit to 1 bit   (STOP = 00)
	bus->CR1 &= ~(USART_CR1_M0 | USART_CR1_M1);
	bus->BRR = uart_clock_speed / baud_rate;
	bus->CR2 &= ~USART_CR2_STOP;

	// Enable
	bus->CR1 |= USART_CR1_UE;	// UART
	bus->CR1 |= USART_CR1_RE;	// Hardware Receiver

	if (rts_cts_control) {
		bus->CR3 |= (USART_CR3_RTSE | USART_CR3_CTSE);
	}

	// Enable Interrupts
	bus->CR1 |= USART_CR1_RXNEIE;	// Receiver Interrupt
//	bus->CR1 |= USART_CR1_RTOIE;	// Timeout Interrupt
}


bool usart_init(USART_TypeDef *bus, int baud_rate) {
	switch((int)bus) {
		case (int)USART1:
			RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
			usart1_gpio_init();
			uart_8bit_1stop(USART3, baud_rate, true);
			break;
		case (int)USART2:
			RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;
			usart2_gpio_init();
			break;
		case (int)USART3:
			RCC->APB1ENR1 |= RCC_APB1ENR1_USART3EN;
			usart3_gpio_init();
			uart_8bit_1stop(USART3, baud_rate, false);
			break;
		case (int)UART4:
			RCC->APB1ENR1 |= RCC_APB1ENR1_UART4EN;
			uart4_gpio_init();
			break;
		case (int)UART5:
			RCC->APB1ENR1 |= RCC_APB1ENR1_UART5EN;
			uart4_gpio_init();
			break;
		case (int)LPUART1:
			RCC->APB1ENR2 |= RCC_APB1ENR2_LPUART1EN;
			lpuart_gpio_init();
			uart_8bit_1stop(LPUART1, baud_rate, false);
			break;
		default:
			return false;
	}
	return true;
}

/**************************** USART TRANSMITTER ****************************/

void usart_transmitChar(USART_TypeDef *bus, char c) {
	// Enable UART3 and Transmitter
	bus->CR1 |= USART_CR1_UE | USART_CR1_TE;

	// Place the character in the Data Register
	bus->TDR = c;

	// Wait for the Transfer to be completed by monitoring the TC flag
	uint64_t start_time = getSysTime(); //time in ms
	while(!(bus->ISR & USART_ISR_TC) && !(is_time_out(start_time, DEFAULT_TIMEOUT_MS)));
}

void usart_transmitBytes(USART_TypeDef *bus, uint8_t message[]) {
	// Enable UART3 and Transmitter
	bus->CR1 |= USART_CR1_UE | USART_CR1_TE;

	// Transfer each character one at a time
	for (int i = 0; i < (int)strlen(message); i++){
		// wait until Data register is empty
		uint64_t start_time = getSysTime(); //time in ms
		while (!(bus->ISR & USART_ISR_TXE) && !(is_time_out(start_time, DEFAULT_TIMEOUT_MS)));
		
		// Place the character in the Data Register
		bus->TDR = message[i];
	}

	// Wait for the Transfer to be completed by monitoring the TC flag
	uint64_t start_time = getSysTime(); //time in ms
	while(!(bus->ISR & USART_ISR_TC) && !(is_time_out(start_time, DEFAULT_TIMEOUT_MS)));
}

/**************************** USART RECEIVER ****************************/

bool usart_receiverTimedOut(USART_ReceiverBuffer *rx) {
	if (rx->timedout) {
		rx->timedout = false;
		return true;
	} else {
		return false;
	}
}

bool usart_receiveBufferNotEmpty(USART_TypeDef *bus) {
	USART_ReceiverBuffer *rxbuff = uart_revisionBusDistinguisher(bus);
	if (rxbuff == NULL) {
		return false;
	}

	return (rxbuff->front != rxbuff->rear);
}

int usart_receiveBytes(USART_TypeDef *bus, uint8_t buffer[], uint16_t size) {
	USART_ReceiverBuffer *rxbuff = uart_revisionBusDistinguisher(bus);
	if (rxbuff == NULL) {
		return false;
	}

	uint64_t start_time = getSysTime(); //time in ms
	uint16_t sz = 0;
	while ((sz < size) && !(is_time_out(start_time, DEFAULT_TIMEOUT_MS))) {
		if (rxbuff->front != rxbuff->rear) {	// rxbuff not empty
			buffer[sz++] = rxbuff->buffer[rxbuff->front];
			rxbuff->front = (rxbuff->front + 1) % ReceiveBufferLen;
		}

		if (usart_receiverTimedOut(rxbuff)) {
			break;
		}
	}

	return sz;
}

/**************************** USART INTERRUPTS ****************************/

void USART1_IRQHandler() {
	if (USART1->ISR & USART_ISR_RXNE) {
		USART1->ISR &= ~USART_ISR_RXNE;
#if OP_REV == 1 || OP_REV == 2
		enqueueBuffer(USART1_RxBuffer, USART1);
#endif
	}
	if (USART1->ISR & USART_ISR_RTOF) {
		USART1->ISR &= ~USART_ISR_RTOF;
#if OP_REV == 1 || OP_REV == 2
		USART1_RxBuffer.timedout = true;
#endif
	}
}

void USART2_IRQHandler() {
	if (USART2->ISR & USART_ISR_RXNE) {
		USART2->ISR &= ~USART_ISR_RXNE;
	}
	if (USART2->ISR & USART_ISR_RTOF) {
		USART2->ISR &= ~USART_ISR_RTOF;
	}
}

void USART3_IRQHandler() {
	if (USART3->ISR & USART_ISR_RXNE) {
		USART3->ISR &= ~USART_ISR_RXNE;
#if OP_REV == 1
		enqueueBuffer(USART3_RxBuffer, USART3);
#endif
	}
	if (USART3->ISR & USART_ISR_RTOF) {
		USART3->ISR &= ~USART_ISR_RTOF;
#if OP_REV == 1
		USART3_RxBuffer.timedout = true;
#endif
	}
}

void UART4_IRQHandler() {
	if (UART4->ISR & USART_ISR_RXNE) {
		UART4->ISR &= ~USART_ISR_RXNE;
	}
	if (UART4->ISR & USART_ISR_RTOF) {
		UART4->ISR &= ~USART_ISR_RTOF;
	}
}

void UART5_IRQHandler() {
	if (UART5->ISR & USART_ISR_RXNE) {
		UART5->ISR &= ~USART_ISR_RXNE;
	}
	if (UART5->ISR & USART_ISR_RTOF) {
		UART5->ISR &= ~USART_ISR_RTOF;
	}
}

void LPUART1_IRQHandler() {
	if (LPUART1->ISR & USART_ISR_RXNE) {
		LPUART1->ISR &= ~USART_ISR_RXNE;
#if OP_REV == 2
		enqueueBuffer(LPUART1_RxBuffer, LPUART1)
#endif
	}
	if (LPUART1->ISR & USART_ISR_RTOF) {
		LPUART1->ISR &= ~USART_ISR_RTOF;
#if OP_REV == 2
		LPUART1_RxBuffer.timedout = true;
#endif
	}
}

