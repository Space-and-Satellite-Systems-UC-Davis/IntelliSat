#ifndef INTELLITASKS_PROTO_H_
#define INTELLITASKS_PROTO_H_

#include "../intelliTask.h"

void task_entry_wrapper(void *params) {
    intelli_task_t *task = (intelli_task_t *)params;
    while (1) {
        if (task->ready_ptr()) {
            task->config_ptr();
            task->run_ptr();
            task->clean_ptr();
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // Small delay to prevent starvation
    }
}

void create_task(intelli_task_t *task) {
    xTaskCreate(
        task_entry_wrapper,  // Wrapper that calls run_ptr()
        task->name,
        configMINIMAL_STACK_SIZE,
        (void*)task,         // Pass task struct to wrapper
        task->func_1,       // Priority
        &task->FreeRTOS_handle
    );
}

/* Scheduling methods */

bool low_pwr_time();
bool detumble_time();
bool comms_time();
bool experiment_time();
bool ecc_time();
bool pizza_time();
bool idle_time();



/* Configure methods */

void config_low_pwr();
void config_detumble();
void config_comms();
void config_experiment();
void config_ecc();
void config_pizza();
void config_idle();


/* Run methods */

void low_pwr();
void detumble();
void comms();
void experiment();
void ecc();
void pizza();
void idle();


/* Clean methods */

void clean_low_pwr();
void clean_detumble();
void clean_comms();
void clean_experiment();
void clean_ecc();;
void clean_pizza();void clean_idle();


#endif//INTELLITASKS_PROTO_H_
