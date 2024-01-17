/*
 * rtc.h
 *
 *  - September 23, 2023
 *  	Author	: Darsh
 *  	Log		: Included configurations
 *
 *  - May 22-25, 2023 (Creation)
 *  	Author : Darsh
 *  	Log    : Wrote the primary rtc interface
 */

#include "rtc.h"

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

void rtc_openWritingPriveledge() {
	// Allow Backup Domain Writing Access
	backup_domain_controlEnable();

	// Enable RTC Write Privilege
	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;

	// Enter RTC Initialization mode, wait for confirmation of initialization mode
	RTC->ISR |= RTC_ISR_INIT;
	while (!(RTC->ISR & RTC_ISR_INITF));
}

void rtc_closeWritingPriveledge() {
	// Exit Initialization Mode
	RTC->ISR &= ~RTC_ISR_INIT;

	// Enable RTC Write Protection
	RTC->WPR = 0xFF;

	// Close Backup Domain Writing Access
	backup_domain_controlDisable();
}

/***************************** RTC CONFIGURATIONS ****************************/

void rtc_config(char clock_source, int forced_config) {
	// do nothing if clock is already configured
	if ((RTC->ISR & RTC_ISR_INITS) && !forced_config) {
		return;
	}

	backup_domain_controlEnable();

	// store the current clock configuration, in case of bad input
	uint32_t temp = RCC->BDCR | RCC_BDCR_RTCSEL;

	// reset the clock
	RCC->BDCR &= ~RCC_BDCR_RTCSEL;

	// Select the RTC clock source
	switch (clock_source) {
		case LSE:
			RCC->BDCR |= RCC_BDCR_RTCSEL_0;
			break;
		case LSI:
			RCC->BDCR |= RCC_BDCR_RTCSEL_1;
			break;
		case HSE:
			RCC->BDCR |= RCC_BDCR_RTCSEL_Msk;
			break;
		default:
			RCC->BDCR |= temp;	// restore the original configuration
			break;
	}

	// Enable the RTC Clock
	RCC->BDCR |= RCC_BDCR_RTCEN;

	backup_domain_controlDisable();

	rtc_openWritingPriveledge();

	// Select the RTC clock source
	switch (clock_source) {
		case LSE:
			RTC->PRER =
				  (127 << RTC_PRER_PREDIV_A_Pos)
				| (255 << RTC_PRER_PREDIV_S_Pos);
			break;
		case LSI:
			RTC->PRER =
				  (127 << RTC_PRER_PREDIV_A_Pos)
				| (249 << RTC_PRER_PREDIV_S_Pos);
			break;
		case HSE:
			// Depends heavily on the clock source
			break;
		default:
			// Let pre-scalers be whatever they are
			break;
	}

	// Bypass the Shadow registers to read RTC directly
	RTC->CR |= RTC_CR_BYPSHAD;

	rtc_closeWritingPriveledge();

}

/****************************** RTC TIME SETTERS *****************************/

void rtc_setCalendar(uint8_t year, uint8_t month, uint8_t date, uint8_t day) {
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

	rtc_openWritingPriveledge();

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

	rtc_closeWritingPriveledge();
}

void rtc_setTime(uint8_t hour, uint8_t minute, uint8_t second) {
	rtc_openWritingPriveledge();

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

	rtc_closeWritingPriveledge();
}

/****************************** RTC TIME GETTERS *****************************/

void rtc_getTime(uint8_t *hour, uint8_t *minute, uint8_t *second) {
	if ((RTC->CR & RTC_CR_BYPSHAD)) {
		// Bypassing Shadow Registers
		// requires reading the registers multiple times

		// temporary variables to hold the second read.
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
