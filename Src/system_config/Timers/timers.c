/*
 * timers.c
 *
 *  - September 23, 2023
 *  	Author	: Darsh
 *  	Log		: ...
 */

#include "../../globals.h"
#include "timers.h"
#include "../LED/led.h"
#include "../../tools/print_scan.h"

// initializing Global (external) variables
int systick_time = 0;
int ag_counter = 0;
int heartbeat_counter = 0;

void timer_init() {
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;

	TIM2->CR1  |= TIM_CR1_URS | TIM_CR1_DIR;
	TIM2->DIER |= TIM_DIER_UIE;

	NVIC_EnableIRQ(TIM2_IRQn);
}

void systick_init() {
	// configure for 1 ms period
	SysTick->LOAD = (core_MHz / 8) * 1000;
	// use AHB/8 as input clock, and enable counter interrupt
	SysTick->CTRL = 0x3;
	NVIC_EnableIRQ(SysTick_IRQn);
}


/**
 * Interrupt handler for the SysTick timer.
 * Increments the systick_time variable and
 * updates the status of the heartbeat and activity LEDs.
 *
 * @param None
 *
 * @returns None
 */
void SysTick_Handler() {
	systick_time++;

	// toggle LEDs
	if (!(systick_time % 1000)) {
		heartbeat_counter = 100;
	}
	if (!(systick_time % 250)) {
		ag_counter = 10;
	}
	if (heartbeat_counter) {
		op_led_hb(1);
		heartbeat_counter--;
	} else {
		op_led_hb(0);
	}
	if (ag_counter) {
		op_led_ag(1);
		ag_counter--;
	} else {
		op_led_ag(0);
	}
}

int num = 0;

void TIM2_IRQHandler() {
	if (TIM2->SR & TIM_SR_UIF) {
		TIM2->SR &= ~TIM_SR_UIF;

		printMsg("%d\n", (num++));
	}
}
