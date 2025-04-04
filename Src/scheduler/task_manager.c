/**
 * @file task_manager.c
 * @brief Logic for task scheduling and preemption.
 *
 * Primary scheduling logic with mode switching, 
 * systems checking and mode selection.
 * 
 * @authors Jacob Tkeio, Nithin Senthil, Parteek Singh
 * @date 1/13/25 updated from 9/8/23
 */

#include "FreeRTOS/Source/include/FreeRTOS.h"
#include "FreeRTOS/Source/include/task.h"

#include "task_manager.h"
#include "intelliTask.h"

#define TASK_MANAGER_YIELD_TICKS pdMS_TO_TICKS(1000) //TODO: temporary 10 seconds 1\10

extern intelli_task_t task_table[6];
const intelli_task_t null_task = (intelli_task_t){TASK_TABLE_LEN, 0, 0, 0, 0, 0, 0, 0, 0};


// /**
//  * @brief Update mode bits
//  *
//  * Checks every mode and updates mode bits
//  * if the mode needs to be scheduled in.
//  *
//  * @see scheduler()
//  * @note Implemented by relevant subteams
//  * @todo Update for combining MRW and HDD
//  *       Double check - mode peripheral requirements
//  */
// void systems_check() {
//     status_check(); // CHARGING flag updated in statusCheck()

//     if (low_pwr_time()) {
//         SET_BIT(flag_bits.mode_bits, LOW_PWR);
//     } else {
//         CLR_BIT(flag_bits.mode_bits, LOW_PWR);
//     }

//     if (detumble_time()) {
//         if (IS_BIT_SET(flag_bits.status_bits, COILS)) {
//                 SET_BIT(flag_bits.mode_bits, DETUMBLE);
//         } else {
//             // printMsg("Coils circuit is not communicating");
//         }
//     } else {
// 	    CLR_BIT(flag_bits.mode_bits, DETUMBLE);
//     }

//     if (comms_time()) {
//         if (IS_BIT_SET(flag_bits.status_bits, ANTENNA)) {
//             SET_BIT(flag_bits.mode_bits, COMMS);
//         } else {
//             // printMsg("Antenna is not deployed\n");
//         }
//     } else {
//         CLR_BIT(flag_bits.mode_bits, COMMS);
//     }

//     int new_exp_id = experiment_time();
//     if (new_exp_id) {
//         SET_BIT(flag_bits.mode_bits, EXPERIMENT);
//         task_table[4].func_1 = new_exp_id;
//     } else {
// 	    CLR_BIT(flag_bits.mode_bits, EXPERIMENT);
//         task_table[4].func_1 = 0;
//     }

//     if (eccTime())
//         SET_BIT(flag_bits.mode_bits, ECC);
//     else
// 	    CLR_BIT(flag_bits.mode_bits, ECC);
    
// }

// /**
//  * @brief Selects highest priority mode
//  *
//  * Checks all the mode bits and selects the highest
//  * priority mode that needs to run.
//  *
//  * @see scheduler()
//  * @todo Update to new idle requirements
//  */
// void mode_select() {

//     // Counters
//     int new_task_id = 0;
//     int i;

//     // Determine which mode to run next
//     for (i = 0; i < 5; i++) {
//         if (IS_BIT_SET(flag_bits.mode_bits, i)) {
//             new_task_id = i;
//             break;
//         }
//     }

//     // Idle mode = CHARGING
//     if (i == 5) {
//         // printMsg("Idle\n");
//         new_task_id = 5;
//     }
    
//     // Select task to run from taskTable 
//     curr_task = task_table[new_task_id];
    
// }


// /**
//  * @brief Handles common tasks during cleanup
//  *
//  * Uses a preference bitfield to determine which cleanup
//  * actions need to be done including logs and clearing
//  * the relevant bit from flag_bits.
//  *
//  * @param field Preference field
//  * @see scheduler()
//  * @todo Update to match logging requirements
//  */
// void cleanup_handler(int8_t field, uint8_t old_task_id) {
//     // b0 - flag_bit reset
//     // b1 - success log

//     if (IS_BIT_SET(field, 0)) {
//         CLR_BIT(flag_bits.mode_bits, task_table[old_task_id].task_id);
//         mode_select();
//     }
//     if (IS_BIT_SET(field, 1)) {
//         // printMsg("Task ID: %d has been logged\n", currTask.task_id);
//     }

//     // currTask.cleanPtr();
//     task_table[old_task_id].clean_ptr();
// }
//


void task_manager(void *args) {
	printMsg(":) Hello from the Task Manager! <3\n\r");

	static intelli_task_t current_task = null_task;
	static uint32_t current_task_dur = 0;

	while(1) {
		printMsg("Running Task Manager\n\r");
		bool cancel_current_task = false;

		// Initi current_task on boot
		if (current_task.id == null_task.id) {
			current_task = task_table[TASK_TABLE_LEN - 1];
		}

		current_task_dur += pdTICKS_TO_MS(TASK_MANAGER_YIELD_TICKS);

		// If expired, cancel. Else if preempted, cancel.
		// TODO: check for current task yield via notif
		if (current_task.timeout != 0 && /* current task has a timeout */
			current_task_dur >= current_task.timeout /* and it expired */
		) {
			cancel_current_task = true;
		} else {
			// Scan for ready task from the next-highest priority downwards
			for (int id = task_table[TASK_TABLE_LEN - 1].id; id > current_task.id; id--) {
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
			if (current_task.id != null_task.id) {
				vTaskSuspend(current_task.FreeRTOS_handle);
				current_task.clean_ptr(); //clean must run after suspend!
				current_task = null_task;
				current_task_dur = 0;
			}

			// Schedule another task, highest priority first
			for (int id = 0; id < TASK_TABLE_LEN; id++) {
				if (task_table[id].ready_ptr()) {
					current_task = task_table[id];
					current_task.config_ptr();
					vTaskResume(current_task.FreeRTOS_handle);
					//TODO: send the new task a start notification
					break;
				}
			}
		}

		vTaskDelay((TickType_t) TASK_MANAGER_YIELD_TICKS);
	}
}
