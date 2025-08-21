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
#define T_LSI (.03125 * 4)
#define IWDG_MAX_TIMEOUT 32768 //ms
#define WWDG_MAX 420 //ms

#define ARR_MAX (1 << 15)
#define MS_TO_S 1000
#define FREQOFF 2 //actualy idk but off by 2

#define WWDG_TIMEOUT_TIME 420 //can change if needed

/**
 * Configure both iwdg and wwdg watchdogs
 * Also initiates hardware interrupt
 * @param ms timeout time in ms, a number between 10 and 420
 * 
 */
void watchdog_init(int ms);

/**
 * Initializes the iwdg
 * 
 * @param ms timeout in ms between 10ms and 32768 (MAX) seconds
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
 * @param ms the time in ms in between interrupts
 */
void watchdog_interrupt_config(int ms);

/**
 * Configures what to do when the interrupt is called
 */
void TIM3_IRQHandler();

/**
 * Configure the IWDG timeout to be the max possible (32 s) for sleep mode (call before entering)
 *
 */
void watchdog_IWDGSleepMode();

/**
 * Function to be called when waking up from sleep, reset the IWDG timeout to what it was before sleeping
 *
 */
void watchdog_IWDGWakeUp();

/**
 * Changes timeout time for IWDG, used for getting in and out of sleep
 * @param ms new time in ms of IWDG
 */
void watchdog_changeIWDGTimeout(int ms);
