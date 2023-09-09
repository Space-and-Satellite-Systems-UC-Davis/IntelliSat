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
#include "./Interrupts/exti_config.h"

/*
 * Configures the system's various features,
 * such as clocks, nvic, gpio, protocol hardware, and more.
 *
 * @param None
 *
 * @returns None
 */
void init_platform() {
	init_clocks();	// initialize the clocks
	init_gpio();	// intitialize the GPIO pins
	EXTI_init();	// configure for GPIO interrupt

	init_nvic();	// initialize the NVIC

	rtc_update_prescaler(0);  // ensure the RTC is working properly
	init_softi2c(OP1_I2C2);	  // initialize the sotfware implemented I2C for I2C Bus 2
	spi2_config();			  // initialize the SPI 2 hardware
	uart_init(USART3, 9600);  // initialize the hardware for USART Bus 3
}
