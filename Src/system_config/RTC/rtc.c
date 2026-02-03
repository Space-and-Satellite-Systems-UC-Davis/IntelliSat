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
 * 
 *  Oct 15, 2025 - Jan 12, 2026 
 * 	Author: Dylan
 * 	Log: Rewrote and created time/date getters
 */

#include "rtc.h"

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
// Callbacks for the timers are stored here.
// Sorted such that earliest entry is the soonest to be called
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
	// Time register
	uint32_t tr_reg = 0;

	// If shadow registers are bypassed
	if (RTC->CR & RTC_CR_BYPSHAD) {
		// If bypassing, read twice to ensure consistent registers
		uint32_t tr_check = 0;
		uint32_t retries = 0;
		const uint32_t max_retries = 10000;

		do {
			tr_reg = RTC->TR;
			tr_check = RTC->TR;
			retries++;
		} while ((tr_reg != tr_check) && (retries < max_retries));
			
	} else {
		// Shadow mode, do single read
		tr_reg = RTC->TR;
	}

	// Converting BCD to binary 
	// Hour
	uint8_t ht = (tr_reg & RTC_TR_HT_Msk) >> RTC_TR_HT_Pos;
	uint8_t hu = (tr_reg & RTC_TR_HU_Msk) >> RTC_TR_HU_Pos;
	*hour = (ht * 10) + hu;
	// Minute
	uint8_t mnt = (tr_reg & RTC_TR_MNT_Msk) >> RTC_TR_MNT_Pos;
	uint8_t mnu = (tr_reg & RTC_TR_MNU_Msk) >> RTC_TR_MNU_Pos;
	*minute = (mnt * 10) + mnu;
	// Second
	uint8_t st = (tr_reg & RTC_TR_ST_Msk) >> RTC_TR_ST_Pos;
	uint8_t su = (tr_reg & RTC_TR_SU_Msk) >> RTC_TR_SU_Pos;
	*second = (st * 10) + su;
}

void rtc_getCalendar(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *day) {
	// Date register
	uint32_t dr_reg = 0;

	// If shadow registers are bypassed
	if (RTC->CR & RTC_CR_BYPSHAD) {
		// If bypassing, read twice to ensure consistent registers
		uint32_t dr_check = 0;
		
		// Retry max amount of times
		// Can maybe swap for is_time_out()
		uint32_t retries = 0; 
		const uint32_t max_retries = 10000; 

		do {
			dr_reg = RTC->DR;
			dr_check = RTC->DR;
			retries++;
		} while ((dr_reg != dr_check) && (retries < max_retries));
			
	} else {
		// else in shadow mode, the shadow registers are frozen during read
		// so single 32-bit read captures the consistent state
		dr_reg = RTC->DR;
	}

	// Converting BCD to binary 
	// Year
	uint8_t yt = (dr_reg & RTC_DR_YT_Msk) >> RTC_DR_YT_Pos;
	uint8_t yu = (dr_reg & RTC_DR_YU_Msk) >> RTC_DR_YU_Pos;
	*year = (yt * 10) + yu;
	// Month 
	uint8_t mt = (dr_reg & RTC_DR_MT_Msk) >> RTC_DR_MT_Pos;
	uint8_t mu = (dr_reg & RTC_DR_MU_Msk) >> RTC_DR_MU_Pos;
	*month = (mt * 10) + mu;
	// Date 
	uint8_t dt = (dr_reg & RTC_DR_DT_Msk) >> RTC_DR_DT_Pos;
	uint8_t du = (dr_reg & RTC_DR_DU_Msk) >> RTC_DR_DU_Pos;
	*date = (dt * 10) + du;

	// Day of week (1 = Monday through 7 = Sunday)
	*day = (dr_reg & RTC_DR_WDU_Msk) >> RTC_DR_WDU_Pos;
}

//Watchdog will be mad if we don't notify it every ~32 seconds
uint32_t sleep_cycles = 0;
uint16_t sleep_remainder = 0;
const uint16_t MAX_SLEEP = 30; // in seconds, must be under 32
bool is_WUTWF_not_ready() { return (RTC->ISR & RTC_ISR_WUTWF) == 0; }

// There are ways to allow it to wait longer if we need to
bool rtc_wakeUp(uint16_t seconds) {
	// Only LSI and LSE will be on for Stop modes.
	// Stop modes not currently implemented
	uint32_t current_source = RCC->BDCR & RCC_BDCR_RTCSEL;
	uint16_t clk_per_sec; //Clock cycles per second
	switch (current_source >> 8) {
		case RTCSEL_LSE:
			clk_per_sec = 2048;
			break;
		case RTCSEL_LSI:
			clk_per_sec = 2000;
			break;
		default:
			return false;
	}

	sleep_cycles = seconds / MAX_SLEEP;
	sleep_remainder = seconds % MAX_SLEEP;

	rtc_openWritingPrivilege();

	RTC->CR &= ~(RTC_CR_WUTE); //clear
	//Can't access WUCKSEL/WUT otherwise
	wait_with_timeout(is_WUTWF_not_ready, DEFAULT_TIMEOUT_MS);

	//Set auto-reload to number of seconds we wait
	RTC->WUTR &= ~(RTC_WUTR_WUT);
	(seconds > MAX_SLEEP) ?
			(RTC->WUTR |= (MAX_SLEEP * clk_per_sec) << RTC_WUTR_WUT_Pos)
			: (RTC->WUTR |= (seconds * clk_per_sec) << RTC_WUTR_WUT_Pos);

	//Select the clock divisor
	//ck_spre appears to cause ~1.3s delay every interrupt?? Avoid
	RTC->CR &= ~(RTC_CR_WUCKSEL);
	RTC->CR |= (WUCKSEL_RTC_DIV16) << RTC_CR_WUCKSEL_Pos;

	//Configure interrupt
	RTC->ISR &= ~(RTC_ISR_WUTF);
    EXTI->IMR1 |= EXTI_IMR1_IM20;
    EXTI->RTSR1 |= EXTI_RTSR1_RT20;
	RTC->CR |= RTC_CR_WUTIE;
    NVIC_EnableIRQ(RTC_WKUP_IRQn);

	RTC->CR |= RTC_CR_WUTE; // Enable wakeup

	rtc_closeWritingPrivilege();

	return true;
}

//Will wake up and turn off itself when done with cycles
void RTC_WKUP_IRQHandler() {
	rtc_openWritingPrivilege();
	//Acknowledged, clear interrupt flags
	RTC->ISR &= ~(RTC_ISR_WUTF);
	EXTI->PR1 |= EXTI_PR1_PIF20;

	//Keep watchdog from resetting
	//"KICK" ;(
	IWDG->KR |= IWDG_KICK;

	if (sleep_cycles == 0 || (sleep_cycles == 1 && sleep_remainder == 0)) {
		RTC->CR &= ~(RTC_CR_WUTE); //Turn off wake-up
	    NVIC_DisableIRQ(RTC_WKUP_IRQn); //Turn off wake-up interrupt
		return; // Do not maintain sleep
	} else if (sleep_cycles == 1) {
		RTC->CR &= ~(RTC_CR_WUTE);
		//Can't access WUCKSEL/WUT otherwise
		wait_with_timeout(is_WUTWF_not_ready, DEFAULT_TIMEOUT_MS);

		uint32_t current_source = RCC->BDCR & RCC_BDCR_RTCSEL;
		uint16_t clk_per_sec; //Clock cycles per second
		switch (current_source >> 8) {
			case RTCSEL_LSE:
				clk_per_sec = 2048;
				break;
			case RTCSEL_LSI:
				clk_per_sec = 2000;
				break;
			default:
				return; //Should never happen
		}

		// Sleep for the remaining seconds
		RTC->WUTR &= ~(RTC_WUTR_WUT);
		RTC->WUTR |= (sleep_remainder * clk_per_sec) << RTC_WUTR_WUT_Pos;
		RTC->CR |= RTC_CR_WUTE;
	}

	sleep_cycles--;
	PWR_maintainLPSleep();

	rtc_closeWritingPrivilege();
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

	uint64_t start_time = getSysTime();
	while(
		callbacks[0].unix_time <= current_unix_time
		&& !is_time_out(start_time, DEFAULT_TIMEOUT_MS)
	) runCurrentTask();

	rtc_closeWritingPrivilege();
}
