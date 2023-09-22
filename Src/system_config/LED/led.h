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

#include "../GPIO/gpio.h"

/***************************** LED INITIALIZERS ******************************/

/*
 * Configures the GPIO ports and pins for the following LEDs:
 * 		D0-D7  ,  A  ,  B  ,  C  ,  AG  ,  HB
 *
 * 	@param   None
 * 	@returns None
 */
void all_led_init();

/******************************* LED TOGGLERS ********************************/

/**
 * Sets the state of LED A.
 *
 * @param   b    The value to set the LED to (0 off or 1 on).
 *
 * @returns None
 */
#define op_led_a(b) gpio_set(GPIOG, 11, value)

/**
 * Sets the state of LED B.
 *
 * @param   b    The value to set the LED to (0 off or 1 on).
 *
 * @returns None
 */
#define op_led_b(b) gpio_set(GPIOG, 12, b)

/**
 * Sets the state of LED C.
 *
 * @param   b    The value to set the LED to (0 off or 1 on).
 *
 * @returns None
 */
#define op_led_c(b) gpio_set(GPIOG, 9, value)

/**
 * Sets the state of the Fault indicator LED.
 *
 * @param   b    The value to set the LED to (0 off or 1 on).
 *
 * @returns None
 */
#define op_led_fault(b) gpio_set(GPIOG, 7, b)

/**
 * Sets the state of the All Good LED
 *
 * @param   b    The value to set the LED to (0 off or 1 on).
 *
 * @returns None
 */
#define op_led_ag(b) gpio_set(GPIOG, 6, b)

/**
 * Sets the state of an Heart Beat LED
 *
 * @param   b    The value to set the LED to (0 off or 1 on).
 *
 * @returns None
 */
#define op_led_hb(b) gpio_set(GPIOE, 2, b)

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
