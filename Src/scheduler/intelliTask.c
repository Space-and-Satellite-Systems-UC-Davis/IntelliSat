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

void send_task_notification(uint8_t task_id, uint32_t bits) {
    for(int i=0; i<TASK_TABLE_LEN; i++) {
        if(task_table[i].id == task_id && task_table[i].FreeRTOS_handle) {
            xTaskNotify(task_table[i].FreeRTOS_handle, bits, eSetBits);
            break;
        }
    }
}

intelli_task_t task_table[TASK_TABLE_LEN] = {
    {6, "LOW_PWR",      60000,  low_pwr_time,    config_low_pwr,    low_pwr,    clean_low_pwr,      NULL, NULL},  // Func1 - N/A
	{5, "DETUMBLE",     60000,  detumble_time,   config_detumble,   detumble,   clean_detumble,     NULL, NULL},  // Func1 - N/A
	{4, "COMMS",        60000,  comms_time,      config_comms,      comms,      clean_comms,        NULL, NULL},  // Func1 - N/A
    {3, "ECC",          60000,  ecc_time,        config_ecc,        ecc,        clean_ecc,          NULL, NULL},  // Func1 - N/A
    {2, "EXPERIMENT",   60000,  experiment_time, config_experiment, experiment, clean_experiment,   NULL, NULL},  // Func1 - Experiment ID (0 for none)
    {1, "PIZZA_TIME",   60000,  pizza_time,      config_pizza,      pizza,      clean_pizza,        NULL, NULL},
    {0, "IDLE",         0,      idle_time,       config_idle,       idle,       clean_idle,         NULL, NULL}   // Func1 - N/A
};
