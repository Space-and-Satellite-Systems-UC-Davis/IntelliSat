/*
 * led.c
 *
 *  - Nov 8-9, 2023
 *      Author       : nithinsenthil
 *      Log          : Updated LED GPIO config for OP Rev2
 *
 *  - Apr 29, 2023 (Creation)
 *      Author       : Tim S.
 *      Contributors : nithinsenthil , Raphael
 *      Log          : Led Control functions written
 */

#include "led.h"

/***************************** LED INITIALIZERS ******************************/

void led_init() {

# if OP_REV == 2

	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOEEN;

	while (GPIOE->OTYPER == 0xFFFFFFFF);

	// Configure output mode
	GPIOE->MODER &= ~(
		  GPIO_MODER_MODE2_Msk
		| GPIO_MODER_MODE3_Msk
		| GPIO_MODER_MODE4_Msk
		| GPIO_MODER_MODE5_Msk
		| GPIO_MODER_MODE6_Msk);

	GPIOE->MODER |=
		  GPIO_MODER_MODE2_0	// Heartbeat
		| GPIO_MODER_MODE3_0	// D3
		| GPIO_MODER_MODE4_0	// D2
		| GPIO_MODER_MODE5_0	// D1
		| GPIO_MODER_MODE6_0;	// D0

# elif OP_REV == 1

	RCC->AHB2ENR |= RCC_AHB2ENR_GPIODEN;
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOEEN;
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOGEN;

	while (GPIOD->OTYPER == 0xFFFFFFFF);
	while (GPIOE->OTYPER == 0xFFFFFFFF);
	while (GPIOG->OTYPER == 0xFFFFFFFF);

	// configure the LED D0-D7 pins to be Output mode
	GPIOD->MODER &= ~(
		  GPIO_MODER_MODE0_Msk
		| GPIO_MODER_MODE1_Msk
		| GPIO_MODER_MODE2_Msk
		| GPIO_MODER_MODE3_Msk
		| GPIO_MODER_MODE4_Msk
		| GPIO_MODER_MODE5_Msk
		| GPIO_MODER_MODE6_Msk
		| GPIO_MODER_MODE7_Msk);
	GPIOD->MODER |=
		  GPIO_MODER_MODE0_0 	// D0
		| GPIO_MODER_MODE1_0	// D1
		| GPIO_MODER_MODE2_0	// D2
		| GPIO_MODER_MODE3_0	// D3
		| GPIO_MODER_MODE4_0	// D4
		| GPIO_MODER_MODE5_0	// D5
		| GPIO_MODER_MODE6_0	// D6
		| GPIO_MODER_MODE7_0;	// D7

	// configure the HEARTBEAT pin to be Output Mode
	GPIOE->MODER &= ~GPIO_MODER_MODE2_Msk;
	GPIOE->MODER |= GPIO_MODER_MODE2_0;

	// Configure the pins on Port G to be in Output Mode
	GPIOG->MODER &= ~(
			  GPIO_MODER_MODE6_Msk
			| GPIO_MODER_MODE7_Msk
			| GPIO_MODER_MODE9_Msk
			| GPIO_MODER_MODE11_Msk
			| GPIO_MODER_MODE12_Msk);
	GPIOG->MODER |=
		  GPIO_MODER_MODE6_0	// LED All Good (AG)
		| GPIO_MODER_MODE7_0	// LED FAULT
		| GPIO_MODER_MODE9_0	// LED C
		| GPIO_MODER_MODE11_0	// LED A
		| GPIO_MODER_MODE12_0;	// LED B

# endif

}

/******************************* LED TOGGLERS ********************************/

#if OP_REV == 1

void led_dx(int pin, int value) {

	if (pin & 0xFFFFFFF8) return;	// not a valid led pin number
	gpio_set(GPIOD, pin, value);

}

#endif

void blinky() {
	static int counter = 0;

	if (counter++ > 1000) {
		counter = 0;
	}
	if (counter > 900) {
		led_hb(1);
	} else {
		led_hb(0);
	}
}
