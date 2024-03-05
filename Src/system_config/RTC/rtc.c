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

void rtc_writeToBKPNumber(uint32_t bits, uint32_t bkp){
		rtc_openWritingPriveledge();
		switch (bkp) {
		    case 0:
		        RTC->BKP0R = bits;
		        break;
		    case 1:
		        RTC->BKP1R = bits;
		        break;
		    case 2:
		        RTC->BKP2R = bits;
		        break;
		    case 3:
		        RTC->BKP3R = bits;
		        break;
		    case 4:
		        RTC->BKP4R = bits;
		        break;
		    case 5:
		        RTC->BKP5R = bits;
		        break;
		    case 6:
		        RTC->BKP6R = bits;
		        break;
		    case 7:
		        RTC->BKP7R = bits;
		        break;
		    case 8:
		        RTC->BKP8R = bits;
		        break;
		    case 9:
		        RTC->BKP9R = bits;
		        break;
		    case 10:
		        RTC->BKP10R = bits;
		        break;
		    case 11:
		        RTC->BKP11R = bits;
		        break;
		    case 12:
		        RTC->BKP12R = bits;
		        break;
		    case 13:
		        RTC->BKP13R = bits;
		        break;
		    case 14:
		        RTC->BKP14R = bits;
		        break;
		    case 15:
		        RTC->BKP15R = bits;
		        break;
		    case 16:
		        RTC->BKP16R = bits;
		        break;
		    case 17:
		        RTC->BKP17R = bits;
		        break;
		    case 18:
		        RTC->BKP18R = bits;
		        break;
		    case 19:
		        RTC->BKP19R = bits;
		        break;
		    case 20:
		        RTC->BKP20R = bits;
		        break;
		    case 21:
		        RTC->BKP21R = bits;
		        break;
		    case 22:
		        RTC->BKP22R = bits;
		        break;
		    case 23:
		        RTC->BKP23R = bits;
		        break;
		    case 24:
		        RTC->BKP24R = bits;
		        break;
		    case 25:
		        RTC->BKP25R = bits;
		        break;
		    case 26:
		        RTC->BKP26R = bits;
		        break;
		    case 27:
		        RTC->BKP27R = bits;
		        break;
		    case 28:
		        RTC->BKP28R = bits;
		        break;
		    case 29:
		        RTC->BKP29R = bits;
		        break;
		    case 30:
		        RTC->BKP30R = bits;
		        break;
		    case 31:
		        RTC->BKP31R = bits;
		        break;
		    default:
		        // Handle case when var is not in the range 0 to 31
		        break;
		}
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
