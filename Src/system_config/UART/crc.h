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
#define MAX_PAYLOAD_BYTES 255
#define CRC_CHECK_SIZE sizeof(uint8_t)
#define MAX_MESSAGE_BYTES MAX_PAYLOAD_BYTES + CRC_CHECK_SIZE

/**
 * Transmit a message. 
 *
 * @param dev
 * @param payload
 * @param nbytes
 */
void crc_transmit(USART_TypeDef *bus, uint8_t *payload, int nbytes);

/**
 * Read a response into `buf`. Assume that `buf` is large enough
 * (MAX_PAYLOAD_BYTES). Returns size of received payload if successfully read,
 * -1 if otherwise.
 */
int crc_read(USART_TypeDef* bus, uint8_t* buf);

#endif /* SYSTEM_CONFIG_UART_CRC_H_ */