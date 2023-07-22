/*
 * experiment_logger.h
 *
 * Change Log:
 *
 * 	- July 22, 2023 (Creation)
 * 		Author : Darsh
 * 		Log    : function declarations based on the initial design plan
 */


// #includes TIMERS, FLASH, RTC, SensorData


/*
 * Each log is a 128 bit word with the following breakdown:
 *      - RTC Time - 12 bits (Min,Sec in binary)
 *      - Gyro     - 48 bits (16 per axis)
 *      - Gyro'    - 48 bits (16 per axis)
 *      - Extra    - 20 bits (potential redundancy)
 *
 * There are 5 such log buffers. Each can hold [INSERT NUMBER HERE] logs.
 */


/**
 * Indicates whether the logger is running or not
 *
 * @returns Boolean (0 or 1)
 */
int is_exp_being_logged();

/**
 * Starts the experiment logger.
 *
 * This function checks makes space in FLASH for the experiment, and records
 * information around that to ensure easy logging.
 * In case of Page overflows, it trashing the previous experiment.
 * Finally, it starts the timer for the
 *
 * @returns None
 */
void start_exp_logging();
// for trashing, use the handle_experiment_overflow() function

/**
 * Stops the experiment logger
 *
 * This function stops the logger timer, and also
 * updates the Experiment header in FLASH
 *
 *`@param experiment_status how the experiment's results must be tagged
 * @returns None
 */
void stop_exp_logging(int experiment_status);

/**
 * Responsible for trashing the oldest experiment
 *
 * This function identifies the oldest experiment in the FLASH,
 * logs critical data about it, and then trashes it by reinitializing it.
 *
 * @returns None
 */
void handle_exp_overflow();

/**
 * Appends an experiment data point
 *
 * This function first detects for an overflow, and handles it.
 * Then it extracts the relevant sensor values, packs them into a log,
 * and then appends that log to the end of the experiment buffer.
 *
 * @returns None
 */
void add_exp_log();
// for handling, use the handle_experiment_buffer_overflow() function

/**
 * Indicates whether the current experiment buffer overflows
 *
 * @returns Boolean (0 or 1)
 */
int detect_exp_buff_overflow();

/**
 * Responsible for handling an experiment buffer overflow
 *
 * THE EXACT POLICT MIGHT CHANGE IN THE FUTURE
 * This function handles an overflow by trashing the last half of the experiment
 *
 * @returns None
 */
void handle_exp_buff_overflow();
