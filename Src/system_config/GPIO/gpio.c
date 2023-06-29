/*
 * gpio.c
 *
 *	- May 11, 2023
 *		Author : Darsh
 *		Log    : Replaced some hardcoded value with placeholders from
 *		         stm32l476xx.h, added commends, and put in code for USART gpio
 *
 *  - Apr 29, 2023 (Creation)
 *      Author       : Tim S.
 *      Contributors : nithinsenthil , Raphael
 *      Log          : All the gpio functions defined.
 */

#include "gpio.h"

/**
 * Initializes the GPIO pins for the leds, buttons, and UART.
 *
 * @param None
 *
 * @returns None
 */
void init_gpio() {
	// enable VDDIO2 supply for GPIOG
	PWR->CR2 |= PWR_CR2_IOSV;

	// wait until each GPIO is clocked and ready
	while (GPIOA->OTYPER == 0xFFFFFFFF);
	while (GPIOB->OTYPER == 0xFFFFFFFF);
	while (GPIOC->OTYPER == 0xFFFFFFFF);
	while (GPIOD->OTYPER == 0xFFFFFFFF);
	while (GPIOE->OTYPER == 0xFFFFFFFF);
	while (GPIOF->OTYPER == 0xFFFFFFFF);
	while (GPIOG->OTYPER == 0xFFFFFFFF);

	/* OP R1 GPIO pinout
	 * 		D7 to D0		D7-D0 	(Output)
	 * 		HEARTBEAT 		E2		(Output)
	 * 		A 				G11		(Output)
	 * 		B 				G12		(Output)
	 * 		C 				G9		(Output)
	 * 		FAULT			G7		(Output)
	 * 		AG 				G6		(Output)
	 * 		BTN1 			B11		(Input)
	 * 		BTN0 			B10		(Input)
	 * 		USART3 TX   	C4		(Alternate Function)
	 * 		USART3 RX   	C5		(Alternate Function)
	 */

	// configure Input mode for BTN0 and BTN1
	GPIOB->MODER = 0x00000000;
	// configure Pull-Up mode for BTN0 and BTN1
	GPIOB->PUPDR =
		  GPIO_PUPDR_PUPD10_0	// BTN0
		| GPIO_PUPDR_PUPD11_0;	// BTN1

	// configure the LED D0-D7 pins to be Output mode
	GPIOD->MODER =
		  GPIO_MODER_MODE0_0 	// D0
		| GPIO_MODER_MODE1_0	// D1
		| GPIO_MODER_MODE2_0	// D2
		| GPIO_MODER_MODE3_0	// D3
		| GPIO_MODER_MODE4_0	// D4
		| GPIO_MODER_MODE5_0	// D5
		| GPIO_MODER_MODE6_0	// D6
		| GPIO_MODER_MODE7_0;	// D7

	// configure the HEARTBEAT pin to be Output Mode
	GPIOE->MODER = GPIO_MODER_MODE2_0;

	// Configure the pins on Port G to be in Output Mode
	GPIOG->MODER =
		  GPIO_MODER_MODE6_0	// LED All Good (AG)
		| GPIO_MODER_MODE7_0	// LED FAULT
		| GPIO_MODER_MODE9_0	// LED C
		| GPIO_MODER_MODE11_0	// LED A
		| GPIO_MODER_MODE12_0;	// LED B

	// configure the USART3 Pins to Alternate Function mode
	GPIOC->MODER &= ~(GPIO_MODER_MODE4_Msk | GPIO_MODER_MODE5_Msk);
	GPIOC->MODER |= (GPIO_MODER_MODE4_1 | GPIO_MODER_MODE5_1);
	// configure the USART3 Pins to be used for UART
	GPIOC->AFR[0] &= ~(GPIO_AFRL_AFSEL4_Msk | GPIO_AFRL_AFSEL5_Msk);
	GPIOC->AFR[0] |= (7U << GPIO_AFRL_AFSEL4_Pos) | (7U << GPIO_AFRL_AFSEL5_Pos);
}

/**
 * Sets the specified GPIO pin to high.
 *
 * @param port The GPIO port to which the pin belongs.
 * @param pin The pin number to set high.
 *
 * @returns None
 */
void gpio_high(GPIO_TypeDef * port, int pin) {
	port->BSRR = 1 << pin;
}

/**
 * Sets the specified GPIO pin to low logic level.
 *
 * @param port The GPIO port to which the pin belongs.
 * @param pin The pin number to set to low.
 *
 * @returns None
 */
void gpio_low(GPIO_TypeDef * port, int pin) {
	port->BSRR = 1 << (pin+16);
}

/**
 * Sets the value of a GPIO pin on a given port.
 *
 * @param port The GPIO port to set the pin on.
 * @param pin The pin number to set.
 * @param value The value to set the pin to (1 for high, 0 for low).
 *
 * @returns None
 */
void gpio_set(GPIO_TypeDef * port, int pin, int value) {
	if (value) {
		gpio_high(port, pin);
	} else {
		gpio_low(port, pin);
	}
}

/**
 * Reads the value of a GPIO pin.
 *
 * @param port The GPIO port to read from.
 * @param pin The pin number to read.
 *
 * @returns The value of the GPIO pin (1 or 0) or -1 if the pin could not be read.
 */
int gpio_read(GPIO_TypeDef * port, int pin) {
	if (port->IDR & (1 << pin)) {
		return 1;
	} else {
		return 0;
	}

	// Prevent from reading junk data if pin is not set to an actual pin
	return -1;
}
