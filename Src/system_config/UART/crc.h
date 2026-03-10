/******************************************************************************
* File:             crc.h
*
* Author:           James Knepper 
* Created:          2025-08-20 1900
* Description:      A simple CRC handler
*
*****************************************************************************/


#ifndef SYSTEM_CONFIG_UART_CRC_H_
#define SYSTEM_CONFIG_UART_CRC_H_

#include <UART/uart.h>

#define POLYAL 0xD8
#define MAX_PAYLOAD_BYTES 254
#define CRC_CHECK_SIZE sizeof(uint8_t)
#define MAX_INTERNAL_BYTES MAX_PAYLOAD_BYTES + CRC_CHECK_SIZE + sizeof(char)
#define MAX_MESSAGE_BYTES ( MAX_INTERNAL_BYTES ) / 2

/**
 * Transmit a message. Assumes the message is small enough
 * (< 128 bytes, can be as large as 253 bytes if the message doesn't require breaks)
 *
 * @param bus
 * @param payload
 * @param nbytes
 * 
 * @returns boolean, whether the message was received (as measured by receipt of an ACK packet)
 */
bool crc_transmit(USART_TypeDef *bus, uint8_t *payload, int nbytes);

/**
 * Read a response into `buf`. Assume that `buf` is large enough
 * (MAX_PAYLOAD_BYTES). Returns size of received payload if successfully read,
 * -1 if otherwise.
 */
int crc_read(USART_TypeDef* bus, uint8_t* buf);

#endif /* SYSTEM_CONFIG_UART_CRC_H_ */