#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "schedulerGlobals.h"

#include "task.h"
#include "status.h"

// extern volatile struct Task currTask;  // volatile: don't cache global var.

// /* Task table */
// extern struct Task taskTable[6];

void block_signal();
void unblock_signal();
bool isSignalBlocked();
void set_user_timeslice(uint32_t t);

void systemsCheck();
void modeSelect();
void cleanup_handler(int8_t field, uint8_t old_task_id);
void scheduler();

//extern bool block_scheduler;
extern jmp_buf to_mode_select;
extern bool block_scheduler;


#endif // SCHEDULER_H
