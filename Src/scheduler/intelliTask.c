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
intelli_task_t task_table[TASK_TABLE_LEN] = {
    {5, "LOW_PWR",      60000,  low_pwr_time,    config_low_pwr,    low_pwr,    clean_low_pwr,      NULL, NULL},  // Func1 - N/A
	{4, "DETUMBLE",     60000,  detumble_time,   config_detumble,   detumble,   clean_detumble,     NULL, NULL},  // Func1 - N/A
	{3, "COMMS",        60000,  comms_time,      config_comms,      comms,      clean_comms,        NULL, NULL},  // Func1 - N/A
    {2, "ECC",          60000,  ecc_time,        config_ecc,        ecc,        clean_ecc,          NULL, NULL},  // Func1 - N/A
    {1, "EXPERIMENT",   60000,  experiment_time, config_experiment, experiment, clean_experiment,   NULL, NULL},  // Func1 - Experiment ID (0 for none)
    {0, "IDLE",         0,      idle_time,       config_idle,       idle,       clean_idle,         NULL, NULL}   // Func1 - N/A
};

/* Scheduling methods */
bool low_pwr_time() {
    // return !(rand() % 101);
    return true;
}
bool detumble_time() {
    // return !(rand() % 4);
    return true;
}
bool comms_time() {
    // return !(rand() % 4);
    return true;
}
int experiment_time() {
    // return !(rand() % 4);
//    return ADCS_recommend_mode();
     return 0;
}
bool ecc_time() {
    // return !(rand() % 4);
    return false;
}
bool idle_time() {
    return true;
}


/* Configure methods */
void config_low_pwr() {
    // Configure method for charging
    // printMsg("Configure Low Power is running\n");
}
void config_detumble() {
    // Configure method for detumble
}
void config_comms() {
    // Configure method for comms
}
void config_experiment() {
    // Configure method for experiment
}
void config_ecc() {
    // Configure method for ecc
}
void config_idle() {
}


/* Run methods */

void low_pwr() {
    // Run method for charging
    // delay_ms(rand());
    // printMsg("Run 'charging'\n");

    led_dx(0, 1);
//    delay_ms( ((rand() % 11) * 100) + 10 );
    delay_ms(1000);
}
    // Run method for detumble
void detumble() {
    // delay_ms(rand());
    // printMsg("Run 'detumble'\n");

    led_dx(1, 1);
//    delay_ms( ((rand() % 11) * 100) + 10 );
    delay_ms(1000);
}
void comms() {
    // Run method for comms
    // printMsg("Run 'comms'\n");

    led_dx(2, 1);
//    delay_ms( ((rand() % 11) * 100) + 10 );
    delay_ms(1000);
}
void experiment() {
    // Run method for experiment
    // printMsg("Run 'experiment'\n");

    led_dx(3, 1);
//    ADCS_MAIN(taskTable[4].func1);

//    delay_ms( ((rand() % 11) * 100) + 10 );
    delay_ms(1000);
}
void ecc() {
    // Run method for ecc
    // printMsg("Run 'ecc'\n");

    led_dx(4, 1);
//    delay_ms( ((rand() % 11) * 100) + 10 );
    delay_ms(1000);
}
void idle() {
    // printMsg("Run Idle\n");

    led_dx(5, 1);
    delay_ms(1000000000000);
    while(1);
}


/* Clean methods */
void clean_low_pwr() {
    // Clean method for charging
    // printMsg("cleanup ID: %d\n", LOWPWR);

    led_dx(0, 0);
}
void clean_detumble() {
    // Clean method for detumble
    // printMsg("cleanup ID: %d\n", DETUMBLE);

    led_dx(1, 0);
}
void clean_comms() {
    // Clean method for comms
    // printMsg("cleanup ID: %d\n", COMMS);

    led_dx(2, 0);
}
void clean_experiment() {
    // Clean method for experiment
    // printMsg("cleanup ID: %d\n", EXPERIMENT);

    led_dx(3, 0);
}
void clean_ecc() {
    // Clean method for ecc
    // printMsg("cleanup ID: %d\n", ECC);

    led_dx(4, 0);
}
void clean_idle() {
    led_dx(5, 0);
}
