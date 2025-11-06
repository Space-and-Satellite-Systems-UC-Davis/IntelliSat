/**
 * @file task.c
 * @brief All task related methods
 *
 * Contains methods pertaining run, clean,
 * and scheduling logic.
 *
 * @authors Nithin Senthil, Parteek Singh, Jacob Tkeo
 * @date 9/8/23
 */

#include "intelliTask.h"
#include "intelliTasks/intelliTasks_proto.h"


/*
    task_table - info. about all modes on satellite
    (mode, timerDur, numInterrupts, & corresponding functions)
*/
/*  LowPower     0		Does the satellite need to enter low power?
	Detumble     1		Does the satellite need to detumble?
	Comms        2		Is there a downlink request?
	ECC          3		Is it time to perform ECC?
	Experiment	 4		Is an experiment ready to run?
	IDLE		 5		Idle when no other mode is running
*/

/*
For quick reference (DO NOT UNCOMMENT)

typedef struct intellisat_task_t {
    uint8_t id;                 // PRIMARY_KEY
    const char *name;		    // Task name as string
    uint32_t timeout;           // time before task times out (ms) 0 means no timeout
    bool (*ready_ptr)(); 	    // returns true when task should be run
    void (*config_ptr)();       // configure timers, other mode info.
    void (*run_ptr)();          // the main func. for mode, via ADCS
    void (*clean_ptr)();        // reset timers, clear temp buffers, etc
    TaskHandle_t FreeRTOS_handle;// FreeRTOS task handle
    uint8_t func_1;             // Open functionality
} intelli_task_t;

*/

intelli_task_t task_table[TASK_TABLE_LEN] = {
    {7, "LOW_PWR",      60000,  low_pwr_time,    config_low_pwr,    low_pwr,    clean_low_pwr,      NULL, NULL},  // Func1 - N/A
	{6, "DETUMBLE",     60000,  detumble_time,   config_detumble,   detumble,   clean_detumble,     NULL, NULL},  // Func1 - N/A
	{5, "COMMS",        60000,  comms_time,      config_comms,      comms,      clean_comms,        NULL, NULL},  // Func1 - N/A
    {4, "ECC",          60000,  ecc_time,        config_ecc,        ecc,        clean_ecc,          NULL, NULL},  // Func1 - N/A
    {3, "EXPERIMENT",   60000,  experiment_time, config_experiment, experiment, clean_experiment,   NULL, NULL},  // Func1 - Experiment ID (0 for none)
    {2, "PIZZA_TIME",   60000,  pizza_time,      config_pizza,      pizza,      clean_pizza,        NULL, NULL},
    {1, "SCHEDIDLE",        0,  idle_time,       config_idle,       idle,       clean_idle,         NULL, NULL}
};
