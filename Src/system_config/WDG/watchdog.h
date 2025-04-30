#include "stm32l476xx.h"
#include <print_scan.h>
#include <globals.h>

#define KEY1 0x45670123
#define KEY2 0xCDEF89AB
#define OPTKEY1 0x08192A3B
#define OPTKEY2 0x4C5D6E7F
#define IWDG_KICK 0x0000AAAA
#define WWDG_KICK WWDG_CR_T_Msk
#define IWDG_START 0x0000CCCC
#define IWDG_ENABLE 0x00005555
#define WWDG_CALCULATE_TIMEOUT 0.0065536
#define APB1_FREQ 5 //MHz
#define WATCHDOG_TIMER TIM3


/**
 * Configure both iwdg and wwdg watchdogs
 * Also initiates hardware interrupt
 * @param ms timeout time in ms, a number between 10 and 420
 * 
 */
void watchdog_config(int ms);

/**
 * Initialize the independent watchdog
 * 
 * @param ms timeout time in ms, a number between 10 and 2048 (2.048 second)
 */
void watchdog_iwdg_config(int ms);

/**
 * Initialize the window watchdog
 * 
 * @param ms timeout time in ms, a number between 10 and 420 
 */
void watchdog_wwdg_config(int ms);

/**
 * Initialize the hardware mode watchdogs
 * 
 * @returns None
 */
void watchdog_hardware_config();

/**
 * Configures the hardware interrupt for TIM3
 * 
 */
void watchdog_interrupt_config();

/**
 * Configures what to do when the interrupt is called
 */
void TIM3_IRQHandler();
/**
 * Periodically kick watchdog inside while loop
 */
void watchdog_kick();