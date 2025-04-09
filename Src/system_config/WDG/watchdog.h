#include "stm32l476xx.h"
#include <print_scan.h>
//#include <globals.h>

#define LSITIME 31.25 //in microseconds
#define KEY1 0x45670123
#define KEY2 0xCDEF89AB
#define OPTKEY1 0x08192A3B
#define OPTKEY2 0x4C5D6E7F

/**
 * Initialize the Independent watchdog
 * 
 * @returns None
 */
void watchdog_config();