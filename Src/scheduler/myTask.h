#ifndef _TASK_H_
#define _TASK_H_

#include "schedulerGlobals.h"
// #include "./../integration/integration.h"

#include "status.h"
#include "FreeRTOS/Source/include/FreeRTOS.h"
#include "FreeRTOS/Source/include/task.h"

#include <stdbool.h>

typedef void (*FunctionPointer)();  // For mode functions

/*
 * Task - a mode of execution on satellite
 * (note: uses preemption to interrupt running tasks)
 */
struct task_t {
    uint8_t task_id;            // PRIMARY_KEY
    const char *task_name;		// Task name as string
    uint32_t task_interrupts;   // times taskISR called, cancel mode/task after x reached
    bool (*ready_ptr)(void); 	// returns true when task should be run
    FunctionPointer config_ptr; // configure timers, other mode info.
    FunctionPointer run_ptr;    // the main func. for mode, via ADCS
    FunctionPointer clean_ptr;  // reset timers, clear temp buffers, etc
    TaskHandle_t FreeRTOS_handle;// FreeRTOS task handle
    uint8_t func_1;             // Open functionality
};

extern volatile struct task_t curr_task;
extern struct task_t task_table[6];

/* Scheduling methods */
bool low_pwr_time(); // tautology (charging is idle mode)
bool detumble_time();
bool comms_time();
int experiment_time();
bool ecc_time();
bool pizza_time();

/* Configure methods */
void config_low_pwr();
void config_detumble();
void config_comms();
void config_experiment();
void config_ecc();
void config_idle();

/* Run methods */
void low_pwr();
void detumble();
void comms();
void experiment();
void ecc();
void idle();

/* Clean methods */
void clean_low_pwr();
void clean_detumble();
void clean_comms();
void clean_experiment();
void clean_ecc();
void clean_idle();

#endif
