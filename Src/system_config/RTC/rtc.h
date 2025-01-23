/*
 * rtc.h
 *
 *  - September 23, 2023
 *  	Author	: Darsh
 *  	Log		: Included configurations
 *
 *  - May 22-23, 2023 (Creation)
 *  	Author : Darsh
 *  	Log    : Wrote the primary rtc interface
 */

#ifndef REALOP1_RTC_H_
#define REALOP1_RTC_H_

#include "stm32l476xx.h"
#include <stdint.h>
#include <core_config.h>

/***************************** RTC CONFIGURATIONS ****************************/

/*
 * Functions to interface with the STM32L476ZG's Internal RTC
 *
 * NOTE: The selection of which oscillator the RTC uses is done through the
 * RCC registers, in rtc_clock_config()
 */

#define LSI 'i'
#define LSE 'l'
#define HSI 'h'
#define HSE 'x'

enum Month{
    January = 1,
    February = 2,
    March = 3,
    April = 4,
    May = 5,
    June = 6,
    July = 7,
    August = 8,
    September = 9,
    October = 10,
    November = 11,
    December = 12
};

enum Weekday{
    Monday = 1,
    Tuesday = 2,
    Wednesday = 3,
    Thursday = 4,
    Friday = 5,
    Saturday = 6,
    Sunday = 7
};

/**
 * Enables the RTC's Clock. Sets the appropriate pre-scalers
 * based on the oscillator source of the RTC.
 * NOTE : The selected Oscillator must be turned on beforehand.
 *
 * @param  clock_source   Predefined options in clock_nvic_config.h
 * @param  forced_config  Setting this to 'false' results in the configuration not taking place in case the RTC is pre-initialized
 *
 * @returns None
 */
void rtc_config(char clock_source, int forced_config);

/**
 * Stores 4 bytes into any of the 32 backup registers on the RTC
 *
 * @param bits 	The bits to be stored
 * @param bkp  	The register to store into (an int between 0-31)
 *
 */
void rtc_writeToBKPNumber(uint32_t bits, uint32_t bkp);

/****************************** RTC TIME SETTERS *****************************/

/**
 * Sets the Year, Month, Date, and Day in the RTC
 * NOTE : rtc.h provides a few definitions to use for 'month' and 'day'
 * NOTE : 'year' refers only to the tens and ones digits, so 23 in 2023
 *
 * @param year   The tens and ones digits of year to be set.
 * @param month  The month to be set. Has usable declarations in rtc.h
 * @param date   The date to be set.
 * @param day    The day to be set. Has usable declarations in rtc.h
 *
 * @returns None
 */
void rtc_setCalendar(uint8_t year, uint8_t month, uint8_t date, uint8_t day);

/**
 * Sets a Time on the RTC
 *
 * @param hour    The hour to be set
 * @param minute  The minute to be set
 * @param second  The second to be set
 * @param None
 *
 * @returns None
 */
void rtc_setTime(uint8_t hour, uint8_t minute, uint8_t second);

/****************************** RTC TIME GETTERS *****************************/

/**
 * Returns the current RTC Time.
 * NOTE : The return values are stored in the function arguments
 *
 * @param hour    Where the function stores the current hour value
 * @param minute  Where the function stores the current minute value
 * @param second  Where the function stores the current second value
 *
 * @returns None
 */
void rtc_getTime(uint8_t *hour, uint8_t *minute, uint8_t *second);


#endif // REALOP1_RTC_H_
