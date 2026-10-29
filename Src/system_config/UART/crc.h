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

/**
 * Send a series of chunks of length <= lchunks
 */
bool crc_chunked_transmit(USART_TypeDef *bus, uint8_t *payload, int nbytes, int lchunks);

/**
 * Read indexed chunks into at most capacity bytes. Returns the actual byte
 * count, or -1 on frame, index or capacity failure. Earlier chunks may have
 * been copied on failure; callers must not use them as a completed transfer.
 * A CRC ACK confirms the frame, not application acceptance or buffer storage.
 */
int crc_chunked_read(USART_TypeDef *bus, uint8_t* buf, int nchunks, size_t capacity);

#endif /* SYSTEM_CONFIG_UART_CRC_H_ */
