/*
 * platform_init.c
 *
 * 	- September 7, 2023
 * 		Author : Darsh
 * 		Log	   : Added spi2_config()
 *
 *	- May 22, 2023
 *		Authod : Darsh
 *		Contributors : Parteek
 *		Log : Backup Power, and RTC Init
 *
 *  - May 11, 2023 (Creation)
 *  	Author : Darsh
 *  	Log    : Created the platform initialization function
 */

#include "platform_init.h"
#include "./UART/uart.h"
#include "./I2C/i2c.h"
#include "./RTC/rtc.h"
#include "./SPI/spi.h"
#include "./Buttons/buttons.h"
#include "./LED/led.h"

/*
 * Configures the system's various features,
 * such as clocks, protocol hardware, and more.
 *
 * @param   None
 * @returns None
 */
void init_platform() {
	init_core_clocks();	// initialize the clocks
	PWR->CR2 |= PWR_CR2_IOSV; // enable VDDIO2 supply for GPIOG

	all_led_init();
	buttons_init();

	init_nvic();	// initialize the NVIC

	rtc_config(LSI, 0);  	  // ensure the RTC is working properly
	init_softi2c(OP1_I2C2);	  // initialize the I2C 2 Bus
	spi2_config();			  // initialize the SPI 2 hardware
	usart_init(USART3, 9600); // initialize the hardware for USART Bus 3
}
