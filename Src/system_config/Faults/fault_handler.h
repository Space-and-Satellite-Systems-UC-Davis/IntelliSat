#ifndef FAULT_HANDLER_H
#define FAULT_HANDLER_H

#include "stm32l476xx.h"
#include "../../tools/print_scan.h"

/**
 * Enable fault handlers by writing to SHCSR 
 * @param none
 *
 * @returns none
 */
void fault_handler_init();


#endif 