/*
 * gpio.c
 *
  *  - September 23, 2023
 *  	Author	: Darsh
 *  	Log		: Removed gpio_init()
 *
 *	- August 28, 2023
 *		Author	: Darsh
 *		Log		: code for SPI-1 GPIO
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
