/**
 * @file task_manager.h
 * @brief Logic for task scheduling and preemption.
 *
 * @authors Jacob Tkeio
 * @date 1/13/25
 */

#ifndef SCHEDULER_TASK_MANAGER_H_
#define SCHEDULER_TASK_MANAGER_H_

#define TASK_MANAGER_PRIORITY 1

/**@brief Handle task expiration and preemption.
 *
 * @param args Unused.
 *
 * @return Void.
 */
void task_manager(void *args);

#endif /* SCHEDULER_TASK_MANAGER_H_ */
