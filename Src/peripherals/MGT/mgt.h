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
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <Timers/timers.h>

#define MGT_USART USART2
#define MGT_BAUDRATE 9600
#define MGT_MAX_STATE_CHANGE_TIME_LIMIT 10
#define BYTE_PER_SECOND_LIMIT 1000
#define MGT_ACKNOWLEDGEMENT_TIMEOUT 1000

const int E_MGT_LOST = 1;
const int E_MGT_INVALID = 2;

enum MgtState { MGT_OFF, MGT_TX_ACTIVE, MGT_RX_ACTIVE};

/* TODO: For testing; remove on deploy <15-01-25, Eric Xu> */
void mgt_transmitBytes(uint8_t message[], int nbytes);
int mgt_readOneByte();
int mgt_readBytes(uint8_t buffer[], int receive_buffer_size);
void mgt_init();
void mgt_sendByteStream(int numberOfBytes, uint8_t buffer[]);
bool mgt_sendByteStreamToMemRequest(int numBytes, uint8_t buffer[]); //TODO: Test
void mgt_sendState(enum MgtState state);
bool mgt_sendTransferToMemRequest(int numBytes); //TODO: Test
char mgt_receiveStateRequest(void);

int mgt_setCoilPercent(uint8_t coilNumber, uint8_t pwm, uint8_t percentage);

#endif /* PERIPHERALS_MGT_MGT_H_ */
