/*
 * radio.h
 *
 *  Created on: Apr 16, 2024
 *      Author: ethansee
 */

#ifndef PERIPHERALS_RADIO_RADIO_H_
#define PERIPHERALS_RADIO_RADIO_H_

#include <globals.h>
#include <stm32l476xx.h>
#include <UART/uart.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <Timers/timers.h>

#define RADIO_USART USART1
#define RADIO_BAUDRATE 9600
#define RADIO_MAX_STATE_CHANGE_TIME_LIMIT 10
#define BYTE_PER_SECOND_LIMIT 1000

enum RadioState { RADIO_STANDBY, RADIO_TX_ACTIVE, RADIO_RX_ACTIVE};

void radio_sendMsg(char* message, ...);
int radio_readOneByte();
void radio_readBytes();
void radio_init();
void radio_sendByte(uint8_t value);
void radio_sendState(enum RadioState state);


#endif /* PERIPHERALS_RADIO_RADIO_H_ */
