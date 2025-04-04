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
	port->BSRR = GPIO_BSRR_BS_ODX_SET << pin; // 15:0 set ODx bit and 
}

void gpio_low(GPIO_TypeDef * port, int pin) {
	port->BSRR = GPIO_BSRR_BR_ODX_RESET << (pin+GPIO_BSRR_BR0_Pos); // 31:16 reset ODx bit
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

void gpio_mode(GPIO_TypeDef * port, int pin, enum gpio_modes mode, int open_drain, int speed, int pull) {
	port->MODER = (port->MODER & ~(GPIO_MODER_Analog << (pin*GPIO_MODER_BITS_SPAN))) | ((mode & GPIO_MODER_Analog) << (pin*GPIO_MODER_BITS_SPAN));
	port->OTYPER = (port->OTYPER & ~(GPIO_OTYPER_OPEN_DRAIN << pin)) | ((open_drain & GPIO_OTYPER_OPEN_DRAIN) << pin);
	port->OSPEEDR = (port->OSPEEDR & ~(GPIO_OSPEEDR_VERY_HIGH << (pin*GPIO_OSPEEDR_BITS_SPAN))) | ((speed & GPIO_OSPEEDR_VERY_HIGH) << (pin*GPIO_OSPEEDR_BITS_SPAN));
	port->PUPDR = (port->PUPDR & ~(GPIO_PUPDR_PUPDx_MSK << (pin*GPIO_PUPDR_BITS_SPAN))) | ((pull & GPIO_PUPDR_PUPDx_MSK) << (pin*GPIO_PUPDR_BITS_SPAN));
}

void gpio_af(GPIO_TypeDef * port, uint8_t pin, uint8_t afn) {
	port->MODER = (port->MODER & ~(GPIO_MODER_Analog << (pin*GPIO_MODER_BITS_SPAN))) | ((GPIO_MODER_AlternateFunction) << (pin*GPIO_MODER_BITS_SPAN));
	if (pin > 7) {
		port->AFR[1] = (port->AFR[1] & ~(GPIO_AFRX_AF15 << ((pin-GPIO_AFRH_AFSEL_PIN_BIT_OFFSET)*GPIO_AFRx_AFSEL_BITS_LEN))) | ((afn & GPIO_AFRX_AF15) << ((pin-GPIO_AFRH_AFSEL_PIN_BIT_OFFSET)*GPIO_AFRx_AFSEL_BITS_LEN));
	} else {
		port->AFR[0] = (port->AFR[0] & ~(GPIO_AFRX_AF15 << (pin*GPIO_AFRx_AFSEL_BITS_LEN))) | ((afn & GPIO_AFRX_AF15) << (pin*GPIO_AFRx_AFSEL_BITS_LEN));
	}
	gpio_low(port, pin);
}
