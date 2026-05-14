/*
 * sleep_test.c
 *
 *  Created on: Aug 12, 2025
 */

#include "sleep.h"
#include "RTC/rtc.h"
#include "WDG/watchdog.h"
#include <print_scan.h>

bool is_DBP_not_set() { return (PWR->CR1 & PWR_CR1_DBP) == 0; }
//Remember that using breakpoints may introduce delay
void testFunction_LPSleep() {
    printMsg("\r\nSleep Test Start. If you are seeing this multiple times, watchdog triggered reset\r\n");

	backup_domain_controlEnable();
	wait_with_timeout(is_DBP_not_set, DEFAULT_TIMEOUT_MS);
	RCC->BDCR |= RCC_BDCR_LSEON;
	backup_domain_controlDisable();

	// Window Watchdog not planned to be used
	WWDG->CR &= ~WWDG_CR_WDGA;

	rtc_config(LSE, true);
    printMsg("\r\nStart LPSleep, LSE, Expected 100 seconds\r\n");
    PWR_enterLPSleepMode(100);
    printMsg("\r\nLPSleep Over\n");

	rtc_config(LSI, true);
	// See http://efton.sk/STM32/gotcha/g200.html on LSI as RTC source
    printMsg("\r\nStart LPSleep, LSI, Expected 100 seconds +/- 3%%\r\n");
    PWR_enterLPSleepMode(100);
    printMsg("\r\nLPSleep Over\n");

	rtc_config(LSE, true);
    printMsg("\r\nStart LPSleep, LSE, Expected 300 seconds\r\n");
    PWR_enterLPSleepMode(300);
    printMsg("\r\nLPSleep Over\n");

    while(true) {
        printMsg("SUCCESS\r\n");
    }
}

void testFunction_mgtSleep() {
    // delay_ms(1000);
	usart_init(USART2, 9600);
	// usart_init(USART1, 9600);
	gpio_mode(GPIOD, 14, GPIO_MODER_Output, GPIO_OTYPER_PUSH_PULL, GPIO_OSPEEDR_HIGH, GPIO_PUPDR_NO_PULL);
	gpio_mode(GPIOD, 3, GPIO_MODER_Output, GPIO_OTYPER_PUSH_PULL, GPIO_OSPEEDR_HIGH, GPIO_PUPDR_NO_PULL);
	gpio_mode(GPIOD, 1, GPIO_MODER_Output, GPIO_OTYPER_PUSH_PULL, GPIO_OSPEEDR_HIGH, GPIO_PUPDR_NO_PULL);
	// infinite loop
	bool valid = true;
    uint8_t c[] = {0, '\0'};
	for(unit8_t i = 10; i < 20; i++) {
		// led_d4(true);
		c[0] = i;
		gpio_low(GPIOD, 14);
		delay_ms(5000);
		gpio_high(GPIOD, 14);
		gpio_low(GPIOD, 3);
		gpio_low(GPIOD, 1);
		// usart_transmitStr(USART2, &c);
		usart_transmitStr(USART2, &c);
		uint8_t c_in[2];
		while(!usart_receiveBufferNotEmpty(USART1));
		usart_receiveBytes(USART2, c_in, 1);
		if(c_in[0] == i + 1) {
			// works, turn on D2
			gpio_high(GPIOD,3);
		}
		else {
			// no work, turn on D3
			gpio_high(GPIOD, 1);
            valid = false;
            break;
		}
	}
    if(valid) {
        while(true) {
            printMsg("Success\r\n");
        }
    }
    else {
        while(true) {
            printMsg("fail\r\n");
        }
    }
}

void testFunction_RadioSleep() {
	usart_init(USART1, 9600);
	gpio_mode(GPIOD, 14, GPIO_MODER_Output, GPIO_OTYPER_PUSH_PULL, GPIO_OSPEEDR_HIGH, GPIO_PUPDR_NO_PULL);
	gpio_mode(GPIOD, 3, GPIO_MODER_Output, GPIO_OTYPER_PUSH_PULL, GPIO_OSPEEDR_HIGH, GPIO_PUPDR_NO_PULL);
	gpio_mode(GPIOD, 1, GPIO_MODER_Output, GPIO_OTYPER_PUSH_PULL, GPIO_OSPEEDR_HIGH, GPIO_PUPDR_NO_PULL);
	// infinite loop
	bool valid = true;
    uint8_t c[] = {0, '\0'};
	for(unit8_t i = 10; i < 20; i++) {
		// led_d4(true);
		c[0] = i;
		gpio_low(GPIOD, 14);
		delay_ms(5000);
		gpio_high(GPIOD, 14);
		gpio_low(GPIOD, 3);
		gpio_low(GPIOD, 1);
		// usart_transmitStr(USART2, &c);
		usart_transmitStr(USART1, &c);
		uint8_t c_in[2];
		while(!usart_receiveBufferNotEmpty(USART1));
		usart_receiveBytes(USART1, c_in, 1);
		if(c_in[0] == i + 1) {
			// works, turn on D2
			gpio_high(GPIOD,3);
		}
		else {
			// no work, turn on D3
			gpio_high(GPIOD, 1);
            valid = false;
            break;
		}
	}
    if(valid) {
        while(true) {
            printMsg("Success\r\n");
        }
    }
    else {
        while(true) {
            printMsg("fail\r\n");
        }
    }
}

