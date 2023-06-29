/*
 * rtc.h
 *
 *  - May 22-23, 2023 (Creation)
 *  	Author : Darsh
 *  	Log    : Wrote the primary rtc interface
 */

#ifndef REALOP1_RTC_H_
#define REALOP1_RTC_H_

#include "stm32l476xx.h"
#include "../clock_nvic_config.h"
#include <stdint.h>

/*
 * Functions to interface with the STM32L476ZG's Internal RTC
 *
 * NOTE: The selection of which oscillator the RTC uses is done through the
 * RCC registers, in system_config/clock_nvic_config : rtc_clock_config()
 */

#define January    1
#define February   2
#define March      3
#define April      4
#define May        5
#define June	   6
#define July       7
#define August     8
#define September  9
#define October    10
#define November   11
#define December   12

#define Monday     1
#define Tuesday    2
#define Wednesday  3
#define Thursday   4
#define Friday     5
#define Saturday   6
#define Sunday     7

/*
 * Sets the appropriate pre-scalers based on the oscillator source of the RTC.
 *
 * @param None
 *
 * @returns None
 */
void rtc_update_prescaler(int forced_config);

/*
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
void rtc_set_calendar(uint8_t year, uint8_t month, uint8_t date, uint8_t day);

/*
 * Sets a Time on the RTC
 *
 * @param hour    The hour to be set
 * @param minute  The minute to be set
 * @param second  The second to be set
 * @param None
 *
 * @returns None
 */
void rtc_set_time(uint8_t hour, uint8_t minute, uint8_t second);

/*
 * Returns the current RTC Time.
 * NOTE : The return values are stored in the function arguements
 *
 * @param hour    Where the function stores the current hour value
 * @param minute  Where the function stores the current minute value
 * @param second  Where the function stores the current second value
 *
 * @returns None
 */
void rtc_get_time(uint8_t *hour, uint8_t *minute, uint8_t *second);


#endif // REALOP1_RTC_H_
