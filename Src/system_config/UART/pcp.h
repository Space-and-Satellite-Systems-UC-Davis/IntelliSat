/******************************************************************************
* File:             pcp.h
*
* Author:           Eric Xu  
* Created:          2025-01-26 20:54
* Description:      Peripheral control protocol (PCP) is a transmission protocol
*                   built above UART that allows the sending and receiving of
*                   packets. Features include
*                   - Buffered send and receive
*                   - Acknowledgement and retransmission
*
*****************************************************************************/


#ifndef SYSTEM_CONFIG_UART_PCP_H_
#define SYSTEM_CONFIG_UART_PCP_H_

#include <globals.h>
#include <stm32l476xx.h>
#include <UART/uart.h>
#include <stdarg.h>
#include <Timers/timers.h>
#include <stdlib.h>

#define E_LOST 1
#define E_INVALID 2
#define E_OVERFLOW 3

// PACKET_START, SEQ_NUM, PACKET_END
#define PCP_HEAD_NBYTES 3
// Number of packets in rx buffer
#define RX_BUFSIZ 32

typedef struct PCPDevice PCPDevice;
// Commented since in progress
//PCPDevice make_pcpdevice(USART_TypeDef *bus,
//                        int timeout_ms,
//                        int outgoing_payload_maxbytes,
//                        int incoming_payload_maxbytes,
//                        int window_size);
int make(PCPDevice* out,
         USART_TypeDef *bus,
         int timeout_ms,
         int outgoing_payload_maxbytes,
         int incoming_payload_maxbytes,
         int window_size);
void delete_members(PCPDevice *dev);

bool request(uint8_t payload[], int nbytes);
int getResponse(uint8_t* buf);
void retransmit(PCPDevice* dev);

#endif /* SYSTEM_CONFIG_UART_PCP_H_ */
