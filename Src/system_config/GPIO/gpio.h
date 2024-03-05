/*
 * gpio.h
 *
 *  - September 23, 2023
 *  	Author	: Darsh
 *  	Log		: Removed gpio_init()
 *
 *  - Apr 29, 2023 (Creation)
 *      Author       : Tim S.
 *      Contributors : nithinsenthil , Raphael
 *      Log          : All the gpio functions defined.
 */

#ifndef REALOP1_GPIO_H_
#define REALOP1_GPIO_H_

#include "stm32l476xx.h"

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
 * 		USART3 TX   	C4		(Alternate Function, AF7)
 * 		USART3 RX   	C5		(Alternate Function, AF7)
 * 		SPI2 CS			B12		(Output)
 * 		SPI2 CLK		B13		(Alternate Function, AF5)
 * 		SPI2 MISO		B14		(Alternate Function, AF5)
 * 		SPI2 MOSI		B15		(Alternate Function, AF5)
 */

/**
 * Sets the specified GPIO pin to high.
 *
 * @param port The GPIO port to which the pin belongs.
 * @param pin The pin number to set high.
 *
 * @returns None
 */
void gpio_high(GPIO_TypeDef * port, int pin);

/**
 * Sets the specified GPIO pin to low logic level.
 *
 * @param port The GPIO port to which the pin belongs.
 * @param pin The pin number to set to low.
 *
 * @returns None
 */
void gpio_low(GPIO_TypeDef * port, int pin);

/**
 * Sets the value of a GPIO pin on a given port.
 *
 * @param port The GPIO port to set the pin on.
 * @param pin The pin number to set.
 * @param value The value to set the pin to (1 for high, 0 for low).
 *
 * @returns None
 */
void gpio_set(GPIO_TypeDef * port, int pin, int value);

/**
 * Reads the value of a GPIO pin.
 *
 * @param port The GPIO port to read from.
 * @param pin The pin number to read.
 *
 * @returns The value of the GPIO pin (1 or 0) or -1 if the pin could not be read.
 */
int gpio_read(GPIO_TypeDef * port, int pin);


#endif /* REALOP1_GPIO_H_ */
