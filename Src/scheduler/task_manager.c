/**
 * @file task_manager.c
 * @brief Logic for task scheduling and preemption.
 *
 * @authors Jacob Tkeio
 * @date 1/13/25
 */

#include "FreeRTOS/Source/include/FreeRTOS.h"
#include "FreeRTOS/Source/include/task.h"

//Assuming task table exists w/ TaskHandle_t
extern struct intellisat_task_t task_table[6];

#define TASK_MANAGER_YIELD_TICKS 10000 //TODO: temporary
#define TASK_TABLE_LENGTH 6 //TODO: temporary


void task_manager(void *args) {
	static intellisat_task_t current_task = task_table[TASK_TABLE_LENGTH - 1]; //idle/null
	static uint32_t current_task_count = 0;

	bool cancel_current_task = false;

	// If expired, cancel. Else if preempted, cancel.
	if (current_task_count++ >= current_task.task_interrupts) {
		cancel_current_task = true;
	} else {
		for (int id = 0; id < current_task.task_id; id++) {
			if ((task_table[id]).ready_ptr()) {
				cancel_current_task = true;
				break;
			}
		}
	}

	if (cancel_current_task) {
		current_task.clean_ptr();
		vTaskDelete(current_task.FreeRTOS_handle);
		task_table[current_task.task_id].FreeRTOS_handle = NULL;
		current_task = 0;
	}

	//schedule other tasks, from high to low priority
	for (int id = 0; id < TASK_TABLE_LENGTH; id++) {
		if (task_table[id].ready_ptr()) {
			//If a task needs to be scheduled and has no handle, create one
			if (!(task_table[id].FreeRTOS_handle)) {
				TaskHandle_t handle = xTaskCreate(/*...*/);
				task_table[id].FreeRTOS_handle = handle;
			}
			//If we need to start a task, choose the highest priority one (it will already be running in FreeRTOS)
			if (!(current_task)) {
				current_task = task_table[id];
				current_task_count = 0;
			}
		}
	}

	vTaskDelay((TickType_t) TASK_MANAGER_YIELD_TICKS);
}
