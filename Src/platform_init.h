/*
 * platform_init.h

 * 	- May 22, 2023
 * 		Author			: Darsh
 * 		Contributors	: Parteek
 * 		Log				: Backup Power, RTC Init
 *
 * 	- May 11, 2023
 * 		Author	: Darsh
 * 		Log		: Created the platform_init() function
 */

#ifndef REALOP1_PLATFORM_INIT_H
#define REALOP1_PLATFORM_INIT_H

#include "system_config/core_config.h"
#include "system_config/Buttons/buttons.h"
#include "system_config/LED/led.h"
#include "system_config/Timers/timers.h"
#include "system_config/RTC/rtc.h"
#include "system_config/UART/uart.h"
#include "system_config/I2C/i2c.h"

/*
 * Configures the system's various features,
 * such as clocks, protocol hardware, and more.
 *
 * @param   None
 * @returns None
 */
void init_platform() {
	init_core_clocks();
	PWR->CR2 |= PWR_CR2_IOSV;

	all_led_init();
	buttons_init();

	systick_init();

	rtc_config(LSI, 0);
	init_softi2c(OP1_I2C2);
	usart_init(USART3, 9600);
}

#endif // REALOP1_PLATFORM_INIT_H
