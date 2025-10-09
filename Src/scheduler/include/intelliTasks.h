#ifndef _TASK_H_
#define _TASK_H_

#include <globals.h>
#include "./../integration/integration.h"
#include "FreeRTOS.h"
#include "task.h"

#ifdef VIRTUAL
#define printMsg printf
#endif

/* Scheduling methods */
bool low_pwr_time();
bool detumble_time();
bool comms_time();
bool experiment_time();
bool pizza_time();

/* Configure methods */
void config_low_pwr();
void config_detumble();
void config_comms();
void config_experiment();
void config_pizza();

/* Run methods */
void low_pwr();
void detumble();
void comms();
void experiment();
void pizza();

/* Clean methods */
void clean_low_pwr();
void clean_detumble();
void clean_comms();
void clean_experiment();
void clean_pizza();

// Debug methods and task
void blinkD2( void *pvParameters );
typedef struct {
    uint8_t led_num;
    uint32_t delay_ms;
} led_task_struct;

void led_task(void*pvParameters);

#endif
