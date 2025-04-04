#ifndef _TASK_H_
#define _TASK_H_

#include "schedulerGlobals.h"
// #include "./../integration/integration.h"

#include "status.h"
#include "FreeRTOS.h"
#include "task.h"

#include <stdbool.h>

#define TASK_TABLE_LEN 6


/*
 * Task - a mode of execution on satellite
 * (note: uses preemption to interrupt running tasks)
 */
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

extern intelli_task_t task_table[TASK_TABLE_LEN];

// /* Scheduling methods */
// bool low_pwr_time(); // tautology (charging is idle mode)
// bool detumble_time();
// bool comms_time();
// int  experiment_time();
// bool ecc_time();
// bool idle_time();
// bool pizza_time();

// /* Configure methods */
// void config_low_pwr();
// void config_detumble();
// void config_comms();
// void config_experiment();
// void config_ecc();
// void config_idle();

// /* Run methods */
// void low_pwr();
// void detumble();
// void comms();
// void experiment();
// void ecc();
// void idle();

// /* Clean methods */
// void clean_low_pwr();
// void clean_detumble();
// void clean_comms();
// void clean_experiment();
// void clean_ecc();
// void clean_idle();

#endif
