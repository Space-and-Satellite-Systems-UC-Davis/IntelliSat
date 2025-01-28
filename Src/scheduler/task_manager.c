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
	// TODO: count ms instead of count*yield_ticks
	// TODO: check for current task yield via notif
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

	if (cancel_current_task == false) {
		vTaskResume(current_task.FreeRTOS_handle);
	} else {
		// Suspend current_task
		vTaskSuspend(current_task.FreeRTOS_handle);
		current_task.clean_ptr(); //clean must run after suspend!
		current_task = NULL;
		current_task_count = 0;

		// Schedule another task, highest priority first
		for (int id = 0; id < TASK_TABLE_LENGTH; id++) {
			if (task_table[id].ready_ptr()) {
				current_task = task_table[id];
				vTaskResume(current_task.FreeRTOS_handle);
				//TODO: send the new task a start notification
				break;
			}
		}
	}

	vTaskDelay((TickType_t) TASK_MANAGER_YIELD_TICKS);
}
