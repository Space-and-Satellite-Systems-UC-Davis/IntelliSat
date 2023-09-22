/*
 * led.h
 *
 *  - Apr 29, 2023 (Creation)
 *      Author       : Tim S.
 *      Contributors : nithinsenthil , Raphael
 *      Log          : Led Control functions written
 */

#ifndef REALOP1_LED_H_
#define REALOP1_LED_H_

#include "../../system_config/GPIO/gpio.h"

/**
 * Sets the state of a LED A connected to a GPIO pin.
 *
 * @param value The value to set the LED to (0 off or 1 on).
 *
 * @returns None
 */
void op_led_a(int value);

/**
 * Sets the state of an LED B connected to a GPIO pin.
 *
 * @param value The value to set the LED to (0 off or 1 on).
 *
 * @returns None
 */
void op_led_b(int value);

/**
 * Sets the state of an LED C connected to a GPIO pin.
 *
 * @param value The value to set the LED to (0 off or 1 on).
 *
 * @returns None
 */
void op_led_c(int value);

/**
 * Sets the state of the LED fault indicator.
 *
 * @param value The value to set the LED fault indicator to (0 off or 1 on).
 *
 * @returns None
 */
void op_led_fault(int value);

/**
 * Sets the state of an LED connected to a GPIO pin.
 *
 * @param value The value to set the LED to (0 off or 1 on).
 *
 * @returns None
 */
void op_led_ag(int value);

/**
 * Sets the state of an Heart Beat LED connected to a GPIO pin.
 *
 * @param value The value to set the LED to (0 off or 1 on).
 *
 * @returns None
 */
void op_led_hb(int value);

/**
 * Sets the value of a specified LED D.
 *
 * @param pin The pin number to set the value for.
 * @param value The value to set the pin to.
 *
 * @returns None
 */
void op_led_dx(int pin, int value);

#endif /* REALOP1_LED_H_ */
