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
#include "intelliTask.h"

#define WATCHDOG_YIELD_TICKS 		pdMS_TO_TICKS(86400000) // 1 day TODO fix
#define WATCHDOG_IDLE_PERIOD_TICKS 	pdMS_TO_TICKS(20000) // 1 minute TODO fix

extern intelli_task_t task_table[TASK_TABLE_LEN];


void watchdog(void *args) {
	printMsg(":) Hello from the Scheduler Watchdog! <3\n\r");

	while (true) {
		static TaskHandle_t task_manager_handle = NULL;

		//Tear down all tasks
		if (task_manager_handle != NULL) {
			vTaskDelete(task_manager_handle);
		}
		for (int taskid = 0; taskid < TASK_TABLE_LEN; taskid++) {
			intelli_task_t task = task_table[taskid];
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
		for (int taskid = 0; taskid < TASK_TABLE_LEN; taskid++) {
			intelli_task_t task = task_table[taskid];
			status = xTaskCreate(	task.run_ptr,
									task.name,
									1000, //TODO: make task-specific
									NULL, //no parameters
									task.id,
									&(task.FreeRTOS_handle)
								);
			if (status != pdPASS) {
				printMsg("OOM allocating %s task with error %d\n\r", task.name, status);
			}
		}

		//Yield to regular scheduling
		vTaskDelay((TickType_t) WATCHDOG_YIELD_TICKS);
	}
}
