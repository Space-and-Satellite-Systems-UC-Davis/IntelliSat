/**
 * @file task_manager.c
 * @brief Logic for task scheduling and preemption.
 *
 * @authors Jacob Tkeio
 * @date 1/13/25
 */

#include "FreeRTOS/Source/include/FreeRTOS.h"
#include "FreeRTOS/Source/include/task.h"

//Assuming task table exists w/ TaskPointer and TaskHandle_t
extern struct intellisat_task_t task_table[6];
//let's make a macro for TASK_TABLE_LENGTH

#define TASK_MANAGER_YIELD_TICKS 10000 //TODO: temporary


void task_manager(void *args) {
	static intellisat_task_t current_task = task_table[TASK_TABLE_LENGTH - 1]; //idle/null

	bool cancel_current_task = false;
	if (/*current_task.expired()???*/true) {
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
	} else {

	}

	for (int id = 0; id <= TASK_TABLE_LENGTH; id++) {
		if (id == TASK_TABLE_LENGTH) { //no other task needs to run

		} else if (task_table[id].ready_ptr()) {
			TaskHandle_t handle = xTaskCreate(/*...*/);
			current_task = task_table[id];
			current_task.handle = handle;
			break;
		}
	}

	//schedule lower priority tasks
	vTaskDelay((TickType_t) TASK_MANAGER_YIELD_TICKS); //TODO check if this is yielding
}
