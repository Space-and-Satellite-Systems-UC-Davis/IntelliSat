// #includes here... FLASH, Scheduler, RTC

/*
 * Each log is a 64 bit word with the following breakdown:
 *      - RTC Date-Time      - 22 bits (Date,Hr,Min,Sec in binary)
 *      - Current Mode       -  4 bits
 *      - Action-Detail Pair - 27 bits (Action-3, Details - 24)
 *      - Extra              - 11 bits (potential redundancy)
 *
 * Details on Action-Detail pairs can be found in the Intellisat Loggers document
 * There is 1 event buffer, which can hold [INSERT NUMBER HERE] logs.
 */

enum Actions {
		// actions here
};


/**
 * Indicates whether the events buffer overflowed
 *
 * @returns Boolean (0 or 1)
 */
int detect_event_overflow();

/**
 * Responsible for handling an event overflow
 *
 * This function handles an overflow by overwriting from the start
 * of the buffer. Also logs the overflow
 *
 * @returns None
 */
void handle_event_overflow();


// NOT PROPERLY DEFINED YET.. They should take in Details as inputs
void log_system_oddity();
void log_intercom();
void log_mode_change();
void log_exp_buffer_overflow();
void log_exp_overflow();
void log_event_overflow();
