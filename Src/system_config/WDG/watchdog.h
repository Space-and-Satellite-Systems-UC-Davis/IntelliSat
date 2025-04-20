#include "stm32l476xx.h"
#include <print_scan.h>
#include <globals.h>

#define KEY1 0x45670123
#define KEY2 0xCDEF89AB
#define OPTKEY1 0x08192A3B
#define OPTKEY2 0x4C5D6E7F
#define IWDG_KICK 0x0000AAAA
#define WWDG_KICK WWDG_CR_T_Msk

typedef struct{
    int* one;
} bad_struct;

/**
 * Initialize the Independent watchdog
 * 
 * @returns None
 */
void watchdog_config();

/**
 * Periodically kick watchdog inside while loop
 */
void watchdog_kick();