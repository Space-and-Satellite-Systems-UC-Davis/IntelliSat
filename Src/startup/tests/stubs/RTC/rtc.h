#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "stm32l476xx.h"

/*
 * Host register model for the RTC/RCC words the Startup interval clock reads.
 * Field order follows the driver's register interface, not STM32 addresses.
 * The legacy backup registers stay modeled so Startup tests can prove that no
 * legacy mission BKP use was reintroduced.
 */
typedef struct {
    volatile uint32_t TR, DR, CR, ISR, PRER, SSR, CALR, WPR;
    volatile uint32_t BKP0R, BKP1R, BKP2R, BKP3R, BKP4R, BKP5R, BKP6R;
} StartupTestRTC;

typedef struct {
    volatile uint32_t BDCR, CSR;
} StartupTestRCC;

extern StartupTestRTC startup_test_rtc;
extern StartupTestRCC startup_test_rcc;
#define RTC (&startup_test_rtc)
#define RCC (&startup_test_rcc)

/* Driver source selectors and calendar mnemonics (Src/system_config/RTC/rtc.h). */
#define LSI 'i'
#define LSE 'l'
#define HSE 'x'
#define January 1
#define Monday 1

enum BKPDesignations {
    BootCounter, ADCSVars, StartupWaitDeadline, StartupFirstRun,
    StartupWaitDone, StartupBurnWireFired, StartupUplinkDeadline
};

/* The existing RTC primitives the interval clock is allowed to call. */
void rtc_config(char clock_source, int forced_config);
void rtc_setCalendar(uint8_t year, uint8_t month, uint8_t date, uint8_t day);
void rtc_setTime(uint8_t hour, uint8_t minute, uint8_t second);
void rtc_getTime(uint8_t *hour, uint8_t *minute, uint8_t *second);
void rtc_getCalendar(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *day);
void rtc_writeToBKPNumber(uint32_t bits, uint32_t bkp);
uint32_t rtc_getUnixTime(void);
