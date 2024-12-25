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

#define GPIO_MODER_BITS_SPAN 2U

#define GPIO_OSPEEDR_BITS_SPAN 2U

#define GPIO_PUPDR_BITS_SPAN 2U

#define GPIO_PUPDR_PUPDx_MSK 3U

#define GPIO_AFRx_AFSEL_BITS_LEN 4U // Bit# of GPIOx_AFRH_AFELx to be set can be obtain by $$(Pin - GPIO_AFRH_AFSEL_PIN_BIT_OFFSET) * GPIO_AFRx_AFSEL_BITS_LEN
#define GPIO_AFRH_AFSEL_PIN_BIT_OFFSET 8U // Bit# of GPIOx_AFRH_AFELx to be set can be obtain by $$(Pin - GPIO_AFRH_AFSEL_PIN_BIT_OFFSET) * GPIO_AFRx_AFSEL_BITS_LEN

enum gpio_modes {
	GPIO_MODER_Input,
	GPIO_MODER_Output,
	GPIO_MODER_AlternateFunction,
	GPIO_MODER_Analog
};

enum gpio_out_states {
	GPIO_OTYPER_PUSH_PULL,
	GPIO_OTYPER_OPEN_DRAIN
};

enum gpio_pull_state {
	GPIO_PUPDR_NO_PULL,
	GPIO_PUPDR_PULL_UP,
	GPIO_PUPDR_PULL_DOWN
};

enum gpio_alternate_func {
	GPIO_AFRX_AF0,
	GPIO_AFRX_AF1,
	GPIO_AFRX_AF2,
	GPIO_AFRX_AF3,
	GPIO_AFRX_AF4,
	GPIO_AFRX_AF5,
	GPIO_AFRX_AF6,
	GPIO_AFRX_AF7,
	GPIO_AFRX_AF8,
	GPIO_AFRX_AF9,
	GPIO_AFRX_AF10,
	GPIO_AFRX_AF11,
	GPIO_AFRX_AF12,
	GPIO_AFRX_AF13,
	GPIO_AFRX_AF14,
	GPIO_AFRX_AF15
};

enum gpio_out_speed {
	GPIO_OSPEEDR_LOW,
	GPIO_OSPEEDR_MED,
	GPIO_OSPEEDR_HIGH,
	GPIO_OSPEEDR_VERY_HIGH
};

enum gpio_odx_set {
	GPIO_BSRR_BS_ODX_DN,
	GPIO_BSRR_BS_ODX_SET
};

enum gpio_odx_reset {
	GPIO_BSRR_BR_ODX_DN,
	GPIO_BSRR_BR_ODX_RESET
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
