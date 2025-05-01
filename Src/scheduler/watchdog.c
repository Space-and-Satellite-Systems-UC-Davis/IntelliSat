/**
 * @file watchdog.c
 * @brief Periodically pull up and tear down scheduler tasks.
 *
 * @author Jacob Tkeio
 * @date 1/27/2025
 */

#include "task_manager.h"
#include "intelliTask.h"

#define WATCHDOG_YIELD_TICKS 		pdMS_TO_TICKS(3000) // 1 day TODO fix 3\86400
#define WATCHDOG_IDLE_PERIOD_TICKS 	pdMS_TO_TICKS(2000) // 1 minute TODO fix 2\20

extern intelli_task_t task_table[TASK_TABLE_LEN];


void watchdog(void *args) {
	printMsg(":) Hello from the Scheduler Watchdog! <3\n\r");

	while (true) {
		printMsg("Running watchdog\r\n");
		static TaskHandle_t task_manager_handle = NULL;

		//Tear down all tasks
		if (task_manager_handle != NULL) {
			vTaskDelete(task_manager_handle);
		}
		for (int taskid = 0; taskid < TASK_TABLE_LEN; taskid++) {
			intelli_task_t task = task_table[taskid];
			if (task.FreeRTOS_handle != NULL) {
				task.clean_ptr();
				vTaskDelete(task.FreeRTOS_handle);
			}
		}

		//Idle to allow FreeRTOS' garbage collection
		vTaskDelay((TickType_t) WATCHDOG_IDLE_PERIOD_TICKS);

		//Pull up all tasks
		BaseType_t status = xTaskCreate( task_manager,
										"Task_Manager",
										1000, //TODO change. # words in stack
										NULL,
										TASK_MANAGER_PRIORITY,
										&task_manager_handle
									);
		if (status != pdPASS) {
			printMsg("OOM allocating task manager task\r\n");
			NVIC_SystemReset(); // If we OOM, we have no recourse but to hard reset
		}
		
		for (int taskid = 0; taskid < TASK_TABLE_LEN; taskid++) {
			intelli_task_t task = task_table[taskid];
			status = xTaskCreate(	task.run_ptr,
									task.name,
									100, //TODO: make task-specific
									NULL, //no parameters
									task.id,
									&(task_table[taskid].FreeRTOS_handle)
								);
			// Make sure the task does not seize control right after \
			// the watchdog and task manager yield
			if (status != pdPASS) {
				vTaskSuspend(task_table[taskid].FreeRTOS_handle);
			} else {
				printMsg("OOM allocating %s task with error %d\n\r", task.name, status);
				NVIC_SystemReset(); // If we OOM, ... (as above)
			}
		}

		//Yield to regular scheduling.
		//Should directly enter the Task Manager task.
		vTaskDelay((TickType_t) WATCHDOG_YIELD_TICKS);
	}
}
