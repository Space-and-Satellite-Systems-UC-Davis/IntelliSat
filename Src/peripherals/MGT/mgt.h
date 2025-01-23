/******************************************************************************
* File:             mgt.h
*
* Author:           Eric Xu  
* Created:          11/03/24 
* Description:      Magnetorquer interface (UART). Based on ethanese's radio
*                   interface
*****************************************************************************/


#ifndef PERIPHERALS_MGT_MGT_H_
#define PERIPHERALS_MGT_MGT_H_

#include <globals.h>
#include <stm32l476xx.h>
#include <UART/uart.h>
#include <stdarg.h>
#include <Timers/timers.h>

#define MGT_USART USART2
#define MGT_BAUDRATE 9600
#define MGT_ACKNOWLEDGEMENT_TIMEOUT 1000

#define E_MGT_LOST 1
#define E_MGT_INVALID 2
#define E_MGT_OVERFLOW 3

#define PAYLOAD_MAXBYTES 125
// PACKET_START, SEQ_NUM, PACKET_END
#define OVERHEAD_MAXBYTES 3
// Without escaping payload
#define PACKET_MAXBYTES PAYLOAD_MAXBYTES + OVERHEAD_MAXBYTES

void mgt_init();
bool mgt_request(uint8_t payload[], int nbytes);
int mgt_getResponse(uint8_t* buf);
void mgt_retransmit();

#endif /* PERIPHERALS_MGT_MGT_H_ */
