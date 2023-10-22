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

void gpio_high(GPIO_TypeDef * port, int pin) {
	port->BSRR = 1 << pin;
}

void gpio_low(GPIO_TypeDef * port, int pin) {
	port->BSRR = 1 << (pin+16);
}

void gpio_set(GPIO_TypeDef * port, int pin, int value) {
	if (value) {
		gpio_high(port, pin);
	} else {
		gpio_low(port, pin);
	}
}

int gpio_read(GPIO_TypeDef * port, int pin) {
	if (port->IDR & (1 << pin)) {
		return 1;
	} else {
		return 0;
	}

	// Prevent from reading junk data if pin is not set to an actual pin
	return -1;
}
