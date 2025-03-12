/******************************************************************************
* File:             mgt.c
*
* Author:           Eric Xu  
* Created:          11/03/24 
* Description:      Communicate to MGT
*****************************************************************************/


#include "MGT/mgt.h"

/**
 * Initialize MGT
 */
void mgt_init() {
	usart_init(MGT_USART, MGT_BAUDRATE);
}

