/*
 * rtc.h
 *
 *  - May 22-25, 2023 (Creation)
 *  	Author : Darsh
 *  	Log    : Wrote the primary rtc interface
 */

#include "rtc.h"

/*
 * Opens the RTC up for making changes to it.
 * NOTE: This action stops the RTC from running, until writing priveledge is closed.
 *
 * @param None
 *
 * @returns None
 */
void rtc_open_writing_priveledge() {
	// Allow Backup Domain Writing Access
	backup_domain_control_enable();

	// Enable RTC Write Priveledge
	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;

	// Enter RTC Initialization mode, wait for confirmation of initialization mode
	RTC->ISR |= RTC_ISR_INIT;
	while (!(RTC->ISR & RTC_ISR_INITF));
}

/*
 * Locks up the RTC so changes can't be made to it. This allows the RTC to start working again.
 *
 * @param None
 *
 * @returns None
 */
void rtc_close_writing_priveledge() {
	// Exit Initialization Mode
	RTC->ISR &= ~RTC_ISR_INIT;

	// Enable RTC Write Protection
	RTC->WPR = 0xFF;

	// Close Backup Domain Writing Access
	backup_domain_control_disable();
}

/*
 * Sets the appropriate pre-scalers based on the oscillator source of the RTC.
 *
 * @param None
 *
 * @returns None
 */
void rtc_update_prescaler(int forced_config) {
	// do nothing if clock is already configured
	if ((RTC->ISR & RTC_ISR_INITS) && !forced_config) {
		return;
	}

	rtc_open_writing_priveledge();

	// check which oscillator the rtc is using,
	// and use formulas from reference manual accordingly
	switch (RCC->BDCR & RCC_BDCR_RTCSEL) {
		case 1:	//LSE
			RTC->PRER =
				  (127 << RTC_PRER_PREDIV_A_Pos)
				| (255 << RTC_PRER_PREDIV_S_Pos);
			break;
		case 2:	// LSI
			RTC->PRER =
				  (127 << RTC_PRER_PREDIV_A_Pos)
				| (249 << RTC_PRER_PREDIV_S_Pos);
			break;
		case 3:	// HSE
			// Depends heavily on the clock source
			break;
		default: // No Clock
			// Let prescalers be whatever they are
			break;
	}

	// Bypass the Shdow registers to read RTC directly
	RTC->CR |= RTC_CR_BYPSHAD;

	rtc_close_writing_priveledge();
}

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
void rtc_set_calendar(uint8_t year, uint8_t month, uint8_t date, uint8_t day) {
	// check to make sure all inputs are valid
	if (year == 0 || year > 99) {
		return;
	}
	if ((month < January) || (month > December)) {
		return;
	}
	if ((date == 0) || (date > 31)) {
		return;
	}
	if ((day < Monday) || (day > Sunday)) {
		return;
	}

	rtc_open_writing_priveledge();

	// reset all the values
	RTC->DR &= ~(
		  RTC_DR_YT_Msk		// Year Tens Digit
		| RTC_DR_YU_Msk		// Year Ones Digit
		| RTC_DR_WDU_Msk	// Weekday Digit
		| RTC_DR_MT_Msk		// Month Tens Digit
		| RTC_DR_MU_Msk		// Month Ones Digit
		| RTC_DR_DT_Msk		// Date Tens Digit
		| RTC_DR_DU_Msk		// Date Ones Digit
	);

	// set the new values
	RTC->DR |= (
		  (year / 10)  << RTC_DR_YT_Pos		// Year Tens Digit
		| (year % 10)  << RTC_DR_YU_Pos		// Year Ones Digit
		| (day)        << RTC_DR_WDU_Pos	// Weekday encoding
		| (month / 10) << RTC_DR_MT_Pos		// Month Tens Digit
		| (month % 10) << RTC_DR_MU_Pos		// Month Ones Digit
		| (date / 10)  << RTC_DR_DT_Pos		// Date Tens Digit
		| (date % 10)  << RTC_DR_DU_Pos		// Date Ones Digit
	);

	rtc_close_writing_priveledge();
}

/*
 * Sets a Time on the RTC
 *
 * @param hour    The hour to be set
 * @param minute  The minute to be set
 * @param second  The second to be set
 *
 * @returns None
 */
void rtc_set_time(uint8_t hour, uint8_t minute, uint8_t second) {
	rtc_open_writing_priveledge();

	// reset all the values
	RTC->TR &= ~(
		  RTC_TR_HT_Msk		// Hour Tens Digit
		| RTC_TR_HU_Msk		// Hour Ones Digit
		| RTC_TR_MNT_Msk	// Minute Tens Digit
		| RTC_TR_MNU_Msk	// Minute Tens Digit
		| RTC_TR_ST_Msk		// Second Tens Digit
		| RTC_TR_SU_Msk		// Second Ones Digit
	);

	// set the new values
	RTC->TR |= (
		  (hour / 10)   << RTC_TR_HT_Pos	// Hour Tens Digit
		| (hour % 10)   << RTC_TR_HU_Pos	// Hour Ones Digit
		| (minute / 10) << RTC_TR_MNT_Pos	// Minute Tens Digit
		| (minute % 10) << RTC_TR_MNU_Pos	// Minute Tens Digit
		| (second / 10) << RTC_TR_ST_Pos	// Second Tens Digit
		| (second % 10) << RTC_TR_SU_Pos	// Second Ones Digit
	);

	rtc_close_writing_priveledge();
}

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
void rtc_get_time(uint8_t *hour, uint8_t *minute, uint8_t *second) {
	if ((RTC->CR & RTC_CR_BYPSHAD)) {
		// Bypassing Shadow Registers
		// requires reading the registers multiple times to ensure consistent values

		// temporary variables to hold the second read.
		// forcefully given these values to enter the while loop
		uint8_t temp_hour   = *hour   + 1;
		uint8_t temp_minute = *minute + 1;
		uint8_t temp_second = *second + 1;

		// check to make sure both values read were consistent
		while ((*hour != temp_hour) || (*minute != temp_minute) || (*second != temp_second)) {
			// read the values once
			*hour    = 10 * ((RTC->TR & RTC_TR_HT_Msk)  >> RTC_TR_HT_Pos);
			*hour   += 1  * ((RTC->TR & RTC_TR_HU_Msk)  >> RTC_TR_HU_Pos);
			*minute  = 10 * ((RTC->TR & RTC_TR_MNT_Msk) >> RTC_TR_MNT_Pos);
			*minute += 1  * ((RTC->TR & RTC_TR_MNU_Msk) >> RTC_TR_MNU_Pos);
			*second  = 10 * ((RTC->TR & RTC_TR_ST_Msk)  >> RTC_TR_ST_Pos);
			*second += 1  * ((RTC->TR & RTC_TR_SU_Msk)  >> RTC_TR_SU_Pos);

			// read the values a second time
			temp_hour    = 10 * ((RTC->TR & RTC_TR_HT_Msk)  >> RTC_TR_HT_Pos);
			temp_hour   += 1  * ((RTC->TR & RTC_TR_HU_Msk)  >> RTC_TR_HU_Pos);
			temp_minute  = 10 * ((RTC->TR & RTC_TR_MNT_Msk) >> RTC_TR_MNT_Pos);
			temp_minute += 1  * ((RTC->TR & RTC_TR_MNU_Msk) >> RTC_TR_MNU_Pos);
			temp_second  = 10 * ((RTC->TR & RTC_TR_ST_Msk)  >> RTC_TR_ST_Pos);
			temp_second += 1  * ((RTC->TR & RTC_TR_SU_Msk)  >> RTC_TR_SU_Pos);
		}
	} else {
		// Not bypassing Shadow Registers
		// simply read the values in the registers
		*hour    = 10 * ((RTC->TR & RTC_TR_HT_Msk)  >> RTC_TR_HT_Pos);
		*hour   += 1  * ((RTC->TR & RTC_TR_HU_Msk)  >> RTC_TR_HU_Pos);
		*minute  = 10 * ((RTC->TR & RTC_TR_MNT_Msk) >> RTC_TR_MNT_Pos);
		*minute += 1  * ((RTC->TR & RTC_TR_MNU_Msk) >> RTC_TR_MNU_Pos);
		*second  = 10 * ((RTC->TR & RTC_TR_ST_Msk)  >> RTC_TR_ST_Pos);
		*second += 1  * ((RTC->TR & RTC_TR_SU_Msk)  >> RTC_TR_SU_Pos);
	}
}
