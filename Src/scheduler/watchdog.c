/**
 * @file watchdog.c
 * @brief Periodically pull up and tear down scheduler tasks.
 *
 * @author Jacob Tkeio
 * @date 1/27/2025
 */

#include "FreeRTOS/Source/include/FreeRTOS.h"
#include "FreeRTOS/Source/include/task.h"

#include "task_manager.h"

extern struct intellisat_task_t task_table[6];
#define TASK_TABLE_LENGTH 6 //TODO: temporary

#define WATCHDOG_YIELD_TICKS 		pdMS_TO_TICKS(86400000) // 1 day TODO fix
#define WATCHDOG_IDLE_PERIOD_TICKS 	pdMS_TO_TICKS(20000) // 1 minute TODO fix


void watchdog(void *args) {
	static TaskHandle_t task_manager_handle = NULL;

	//Tear down all tasks
	if (task_manager_handle != NULL) {
		vTaskDelete(task_manager_handle);
	}
	for (int taskid = 0; taskid < TASK_TABLE_LENGTH; taskid++) {
		task_t task = task_table[taskid];
		if (task.FreeRTOS_handle != NULL) {
			vTaskDelete(task.FreeRTOS_handle);
		}
	}

	//Idle to allow FreeRTOS' garbage collection
	vTaskDelay((TickType_t) WATCHDOG_IDLE_PERIOD_TICKS);

	//Pull up all tasks
	BaseType_t status = xTaskCreate( task_manager,
			                     	 ":) Hello from the Task Manager! <3",
									 1000, //TODO change. # words in stack
									 NULL,
									 TASK_MANAGER_PRIORITY,
									 &task_manager_handle
			                       );
	if (status != pdPASS) {
		printMsg("OOM allocating task manager task\n");
	}
	for (int taskid = 0; taskid < TASK_TABLE_LENGTH; taskid++) {
		task_t task = task_table[taskid];
		status = xTaskCreate(	task.run_ptr,
								task.task_name,
								1000, //TODO: make task-specific
								NULL, //no parameters
								task.task_id,
								&(task.FreeRTOS_handle)
							);
		if (status != pdPASS) {
			printMsg("OOM allocating %s task\n", task.task_name);
		}
	}

	//Yield to regular scheduling
	vTaskDelay((TickType_t) WATCHDOG_YIELD_TICKS);
}
