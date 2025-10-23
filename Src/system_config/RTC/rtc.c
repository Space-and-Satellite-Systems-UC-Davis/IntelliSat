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
// Callbacks for the timers are stored here.
// Sorted such that earliest entry is the soonest to be called
// TODO: Track length?
CallbackEntry callbacks[TIMER_CALLBACK_ARRAY_SIZE];

//Incremeted every time a callback is added to be unique. Not index.
uint32_t id_counter = 0;

void init_callbacks() {
	for (int i = 0; i < TIMER_CALLBACK_ARRAY_SIZE; i++) {
		CallbackEntry dummy_entry;
		dummy_entry.id = NULL_ID;
		callbacks[i] = dummy_entry;
	}
}

bool is_not_init_RTC() { return !(RTC->ISR & RTC_ISR_INITF); }
void rtc_openWritingPrivilege() {
	// Allow Backup Domain Writing Access
	backup_domain_controlEnable();

	// Enable RTC Write Privilege
	RTC->WPR = RTC_WPR_WRITE_PROTECT_ON_1;
	RTC->WPR = RTC_WPR_WRITE_PROTECT_ON_2;

	// Enter RTC Initialization mode, wait for confirmation of initialization mode
	RTC->ISR |= RTC_ISR_INIT;
	wait_with_timeout(is_not_init_RTC, DEFAULT_TIMEOUT_MS);
}

void rtc_closeWritingPrivilege() {
	// Exit Initialization Mode
	RTC->ISR &= ~RTC_ISR_INIT;

	// Enable RTC Write Protection
	RTC->WPR = RTC_WPR_WRITE_PROTECT_OFF;

	// Close Backup Domain Writing Access
	backup_domain_controlDisable();
}

/***************************** RTC CONFIGURATIONS ****************************/

bool is_BDRST_not_set() { return (RCC->BDCR & RCC_BDCR_BDRST) == 0; }
void rtc_config(char clock_source, int forced_config) {
	// do nothing if clock is already configured
	if ((RTC->ISR & RTC_ISR_INITS) && !forced_config) {
		return;
	}

	backup_domain_controlEnable();

	// store the current clock configuration, in case of bad input
	uint32_t backup = RCC->BDCR & ~RCC_BDCR_RTCSEL;
	uint32_t backup_rtcsel = RCC->BDCR & RCC_BDCR_RTCSEL;

	// reset the clock
	RCC->BDCR |= RCC_BDCR_BDRST;
	wait_with_timeout(is_BDRST_not_set, DEFAULT_TIMEOUT_MS);
	RCC->BDCR &= ~RCC_BDCR_BDRST;

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
			RCC->BDCR |= backup_rtcsel;	// restore the original configuration
			break;
	}

	//Restore from reset
	RCC->BDCR |= backup;
	// Enable the RTC Clock
	RCC->BDCR |= RCC_BDCR_RTCEN;

	backup_domain_controlDisable();

	rtc_openWritingPrivilege();

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

	init_callbacks();

	rtc_closeWritingPrivilege();

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

	rtc_openWritingPrivilege();

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

	rtc_closeWritingPrivilege();
}

void rtc_setTime(uint8_t hour, uint8_t minute, uint8_t second) {
	rtc_openWritingPrivilege();

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

	rtc_closeWritingPrivilege();
}

void rtc_writeToBKPNumber(uint32_t bits, uint32_t bkp){
		rtc_openWritingPrivilege();
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
		rtc_closeWritingPrivilege();
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
		uint64_t start_time = getSysTime(); //time in ms
		while (
			((*hour != temp_hour) || 
			(*minute != temp_minute) || 
			(*second != temp_second))
//			&& is_time_out(start_time, DEFAULT_TIMEOUT_MS)
		) {
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

/************************** RTC SCHEDULE CALLBACK *****************************/

// We only use Alarm A
// Sets alarm for whatever callbacks[0] is at the time
void setAlarm() {
	rtc_openWritingPrivilege();

	const CallbackEntry entry = callbacks[0];
	if (entry.id == NULL_ID) return;

    // Disable alarm
	// Needs to be done to edit
	RTC->CR &= ~RTC_CR_ALRAE;

	// Configure interrupt
	RTC->ISR &= ~(RTC_ISR_ALRAF);
	RTC->CR |= RTC_CR_ALRAIE;
    EXTI->IMR1 |= EXTI_IMR1_IM18;
    EXTI->RTSR1 |= EXTI_RTSR1_RT18;
    NVIC_EnableIRQ(RTC_Alarm_IRQn);

    // Calculate when to set alarm
    uint32_t unix_time = entry.unix_time;

    //We assume we don't get more than delta 24 hours for now
    uint8_t adjusted_hours = unix_time / (60 * 60);
    unix_time -= (adjusted_hours * 60 * 60);

    uint8_t adjusted_minutes = unix_time / 60;
    unix_time -= (adjusted_minutes * 60);

    uint8_t adjusted_seconds = unix_time;

    // Program alarm
	RTC->ALRMAR = 0;
	RTC->ALRMAR |= (
		  (adjusted_hours / 10)   << RTC_ALRMAR_HT_Pos	// Hour Tens Digit
		| (adjusted_hours % 10)   << RTC_ALRMAR_HU_Pos	// Hour Ones Digit
		| (adjusted_minutes / 10) << RTC_ALRMAR_MNT_Pos	// Minute Tens Digit
		| (adjusted_minutes % 10) << RTC_ALRMAR_MNU_Pos	// Minute Tens Digit
		| (adjusted_seconds / 10) << RTC_ALRMAR_ST_Pos	// Second Tens Digit
		| (adjusted_seconds % 10) << RTC_ALRMAR_SU_Pos	// Second Ones Digit
	);
	// For some reason the initial date is not the same as the date we get in ALRMAR
	RTC->ALRMAR |= RTC_ALRMAR_MSK4;

    // Enable alarm
	RTC->CR |= RTC_CR_ALRAE;

	rtc_closeWritingPrivilege();
}

// Assumes there is empty space
int compareEntry(const void* a, const void* b) {
	CallbackEntry entry_a = * ( (CallbackEntry*) a );
	CallbackEntry entry_b = * ( (CallbackEntry*) b );

    if ( entry_a.unix_time == entry_b.unix_time ) return 0;
    else if ( entry_a.unix_time < entry_b.unix_time ) return -1;
    else return 1;
}

// We need to sort and setAlarm to keep the alarm at the earliest callback
uint32_t rtc_insertEntry(CallbackEntry entry) {
	for (int i = 0; i < TIMER_CALLBACK_ARRAY_SIZE; i++) {
		if (callbacks[i].id == NULL_ID) {
			entry.id = id_counter;
			callbacks[i] = entry;

			qsort(
				callbacks,
				TIMER_CALLBACK_ARRAY_SIZE,
				sizeof(CallbackEntry),
				compareEntry
			);

			setAlarm();

			return id_counter++;
		}
	}

	return NULL_ID;
}

bool rtc_deleteEntry(uint32_t id) {
	for (int i = 0; i < TIMER_CALLBACK_ARRAY_SIZE; i++) {
		if (callbacks[i].id == id) {
			callbacks[i].id = NULL_ID;

			// Need to do for sorting
			callbacks[i].unix_time = NULL_UNIX_TIME;

			qsort(
				callbacks,
				TIMER_CALLBACK_ARRAY_SIZE,
				sizeof(CallbackEntry),
				compareEntry
			);

			setAlarm();

			return true;
		}
	}

	return false;
}
void rtc_deleteAllEntries() {
	for (int i = 0; i < TIMER_CALLBACK_ARRAY_SIZE; i++) {
		callbacks[i].id = NULL_ID;
		callbacks[i].unix_time = NULL_UNIX_TIME;
	}
}

CallbackEntry rtc_getEntry(uint32_t id) {
	for(int i = 0; i < TIMER_CALLBACK_ARRAY_SIZE; i++) {
		if (callbacks[i].id == id) return callbacks[i];
	}

	CallbackEntry dummy_entry;
	dummy_entry.id = NULL_ID;
	return dummy_entry;
}
bool rtc_isEntryActive(uint32_t id) {
	return rtc_getEntry(id).id != NULL_ID;
}

uint32_t getUnixTime(
		uint32_t d_seconds,
		uint32_t d_minutes,
		uint32_t d_hours
) {
    uint32_t unix_time = 0;
    unix_time += d_seconds;
    unix_time += d_minutes * 60;
    unix_time += d_hours * 60 * 60;

    return unix_time;
}

// Manages our callback state and then sets alarm
uint32_t rtc_scheduleCallback(
		uint8_t d_seconds,
		uint8_t d_minutes,
		uint8_t d_hours,
		bool continuous,
		timer_callback callback
) {
	CallbackEntry entry;
	entry.callback = callback;
	entry.next_time = 0;
	if (continuous) {
		entry.next_time = getUnixTime(d_seconds, d_minutes, d_hours);
	}

	// Program time to trigger
    uint8_t hour, minute, second;
    rtc_getTime(&hour,&minute, &second);

    entry.unix_time = getUnixTime(
    		second + d_seconds,
			minute + d_minutes,
			hour + d_hours
    );

    uint32_t id = rtc_insertEntry(entry);
    //TODO: Proper error handling
    if (id == NULL_ID) {
    	return NULL_ID;
    }

    return id;
}

void runCurrentTask() {
	CallbackEntry entry = callbacks[0];
	entry.callback();
	rtc_deleteEntry(entry.id);

	if (entry.next_time != 0) {
		entry.unix_time += entry.next_time;
		rtc_insertEntry(entry);
	}
}

void RTC_ALARM_IRQHandler() {
	rtc_openWritingPrivilege();
	//Acknowledged, clear interrupt flags
	RTC->ISR &= ~(RTC_ISR_ALRAF);
	EXTI->PR1 |= EXTI_PR1_PIF18;

	// If task has been deleted, stop here
	if (callbacks[0].id == NULL_ID) {
		rtc_closeWritingPrivilege();
		return;
	}

	runCurrentTask();

	//In case next task should have already been run
    uint8_t hour, minute, second;
    rtc_getTime(&hour,&minute, &second);
    uint32_t current_unix_time = getUnixTime(second, minute, hour);

    //TODO: Timeout
	while(callbacks[0].unix_time <= current_unix_time) runCurrentTask();

	rtc_closeWritingPrivilege();
}

