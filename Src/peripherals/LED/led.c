/*
 * led.c
 *
 *  - Apr 29, 2023 (Creation)
 *      Author       : Tim S.
 *      Contributors : nithinsenthil , Raphael
 *      Log          : Led Control functions written
 */

#include "led.h"

/**
 * Sets the state of a LED A connected to a GPIO pin.
 *
 * @param value The value to set the LED to (0 off or 1 on).
 *
 * @returns None
 */
void op_led_a(int value) {
	gpio_set(GPIOG, 11, value);
}

/**
 * Sets the state of an LED B connected to a GPIO pin.
 *
 * @param value The value to set the LED to (0 off or 1 on).
 *
 * @returns None
 */
void op_led_b(int value) {
	gpio_set(GPIOG, 12, value);
}

/**
 * Sets the state of an LED C connected to a GPIO pin.
 *
 * @param value The value to set the LED to (0 off or 1 on).
 *
 * @returns None
 */
void op_led_c(int value) {
	gpio_set(GPIOG, 9, value);
}

/**
 * Sets the state of the LED fault indicator.
 *
 * @param value The value to set the LED fault indicator to (0 off or 1 on).
 *
 * @returns None
 */
void op_led_fault(int value) {
	gpio_set(GPIOG, 7, value);
}

/**
 * Sets the state of an LED connected to a GPIO pin.
 *
 * @param value The value to set the LED to (0 off or 1 on).
 *
 * @returns None
 */
void op_led_ag(int value) {
	gpio_set(GPIOG, 6, value);
}

/**
 * Sets the state of an Heart Beat LED connected to a GPIO pin.
 *
 * @param value The value to set the LED to (0 off or 1 on).
 *
 * @returns None
 */
void op_led_hb(int value) {
	gpio_set(GPIOE, 2, value);
}

/**
 * Sets the value of a specified LED D.
 *
 * @param pin The pin number to set the value for.
 * @param value The value to set the pin to.
 *
 * @returns None
 */
void op_led_dx(int pin, int value) {
	if (pin & 0xFFFFFF00) return;	// not a valid D led pin numebr
	gpio_set(GPIOD, pin, value);
}
