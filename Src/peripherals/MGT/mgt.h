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

void mgt_init();

#endif /* PERIPHERALS_MGT_MGT_H_ */
