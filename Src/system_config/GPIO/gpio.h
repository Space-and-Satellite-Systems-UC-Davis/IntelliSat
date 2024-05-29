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

enum gpio_modes {
	GPIO_Input,
	GPIO_Output,
	GPIO_AlternateFunction,
	GPIO_Analog
};

enum gpio_pull_states {
	GPIO_PullupPulldown,
	GPIO_Pullup,
	GPIO_Pulldown,
};

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

/*
 * Sets the mode of a certain GPIO pin
 *
 * @param port        The GPIO port to configure.
 * @param pin         The pin number of the GPIO pin to configure.
 * @param mode        The mode for the GPIO pin (check gpio.h for the enum)
 * @param open_drain  0 for Normal Push-Pull, 1 for Open-Drain
 * @param speed		  0x3 for very high speed, 0x0 for normal low speed
 * @param pull		  The state of the GPIO pin (check gpio.h for the enum)
 */
void gpio_mode(GPIO_TypeDef * port, int pin, enum gpio_modes mode, int open_drain, int speed, int pull);

/*
 * Configures a certain GPIO pin for Alternate Function mode
 *
 * @param port  The GPIO port to configure.
 * @param pin   The pin number of the GPIO pin to configure.
 * @param afn	The Alternate Function number for the GPIO pin
 */
void gpio_af(GPIO_TypeDef * port, uint8_t pin, uint8_t afn);


#endif /* REALOP1_GPIO_H_ */
