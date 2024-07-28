/**
 * @file scheduler.c
 * @brief Scheduler logic
 *
 * Primary scheduling logic with mode switching, 
 * systems checking and mode selection.
 *
 * @authors Nithin Senthil, Parteek Singh, Jacob Tkeo
 * @date 9/8/23
 */

#include "scheduler.h"

#define SYSTICK_DUR_M 3     // Config. of system timer in usec (100 ms)

/*
 * taskTable - info. about all modes on satellite
 * (mode, timerDur, numInterrupts, & corresponding functions)
 */

// TODO: update ECC mode based on discussions with ELEC
// RS - system, Hamming codes on Idle
struct Task taskTable[] = {
	{LOWPWR, 10000, configLowPwr, lowPwr, cleanLowPwr, 0},                  // Func1 - N/A
	{DETUMBLE, 5, configDetumble, detumble, cleanDetumble, 0},           // Func1 - N/A
	{COMMS, 5, configComms, comms, cleanComms, 0},                        // Func1 - N/A
    {ECC, 200, configEcc, ecc, cleanEcc, 0},                                // Func1 - N/A
    {EXPERIMENT, 200, configExperiment, experiment, cleanExperiment, 0},    // Func1 - Experiment ID (0 for none)
    {IDLE, 200, configIdle, idle, cleanIdle, 0}                             // Func1 - N/A
};

volatile struct Task currTask;

uint32_t user_timeslice;
int scheduler_counter;
u_int32_t task_counter;

jmp_buf to_mode_select;


//  /**
//  * @brief Signal block
//  *
//  * Blocks the specified interrupt signal by
//  * by adding it to the curr. process blocked signals
//  *
//  * @see unblock_signal(), isSignalBlocked()
//  * @warning Make sure a corresponding unblock is in place
//  */
// void block_signal() {
//     block_scheduler = true;
// }


//   /**
//  * @brief Signal unblock
//  *
//  * Unblocks the specified interrupt signal by
//  * by removing it from the curr. process blocked signals
//  *
//  * @see block_signal(), isSignalBlocked()
//  */
// void unblock_signal() {
//     block_scheduler = false;
// }

//  /**
//  * @brief Checks if signal is blocked
//  *
//  * Checks if the specified signal is
//  * on the curr. process blocked signals
//  *
//  * @param signal Signal to check
//  * @see unblock_signal(), block_signal()
//  */
// bool isSignalBlocked() {
//     return block_scheduler;
// }

/**
 * @brief Timeslice setter
 *
 * Sets the timeslice used in order to pre-empt the
 * runnning task.
 *
 * @param t timeslice in ms
 * @note Inputs must be a multiple of 10
 */
void set_user_timeslice(uint32_t t) {
    user_timeslice = t;
}


/**
 * @brief Update mode bits
 *
 * Checks every mode and updates mode bits
 * if the mode needs to be scheduled in.
 *
 * @see scheduler()
 * @note Implemented by relevant subteams
 * @todo Update for combining MRW and HDD
 *       Double check - mode peripheral requirements
 */
void systemsCheck() {
    statusCheck(); // CHARGING flag updated in statusCheck()

    if (lowPwrTime()) {
        SET_BIT(flagBits.modeBits, LOWPWR);
    } else {
        CLR_BIT(flagBits.modeBits, LOWPWR);
    }

    if (detumbleTime()) {
        if (IS_BIT_SET(flagBits.statusBits, COILS)) {
                SET_BIT(flagBits.modeBits, DETUMBLE);
        } else {
            // printMsg("Coils circuit is not communicating");
        }
    } else {
	    CLR_BIT(flagBits.modeBits, DETUMBLE);
    }

    if (commsTime()) {
        if (IS_BIT_SET(flagBits.statusBits, ANTENNA)) {
            SET_BIT(flagBits.modeBits, COMMS);
        } else {
            // printMsg("Antenna is not deployed\n");
        }
    } else {
        CLR_BIT(flagBits.modeBits, COMMS);
    }

    int newExpID = experimentTime();
    if (newExpID) {
        SET_BIT(flagBits.modeBits, EXPERIMENT);
        taskTable[4].func1 = newExpID;
    } else {
	    CLR_BIT(flagBits.modeBits, EXPERIMENT);
        taskTable[4].func1 = 0;
    }

    if (eccTime())
        SET_BIT(flagBits.modeBits, ECC);
    else
	    CLR_BIT(flagBits.modeBits, ECC);
    
}


/**
 * @brief Selects highest priority mode
 *
 * Checks all the mode bits and selects the highest
 * priority mode that needs to run.
 *
 * @see scheduler()
 * @todo Update to new idle requirements
 */
void modeSelect() {

    // Counters
    int new_task_id = 0;
    int i;

    // Determine which mode to run next
    for (i = 0; i < 5; i++) {
        if (IS_BIT_SET(flagBits.modeBits, i)) {
            new_task_id = i;
            break;
        }
    }

    // Idle mode = CHARGING
    if (i == 5) {
        // printMsg("Idle\n");
        new_task_id = 5;
    }
    
    // Select task to run from taskTable 
    currTask = taskTable[new_task_id];

}

/**
 * @brief Handles common tasks during cleanup
 *
 * Uses a preference bitfield to determine which cleanup
 * actions need to be done including logs and clearing
 * the relevant bit from flagBits.
 *
 * @param field Preference field
 * @see scheduler()
 * @todo Update to match logging requirements
 */
void cleanup_handler(int8_t field, uint8_t old_task_id) {
    // b0 - flagBit reset
    // b1 - success log

    if (IS_BIT_SET(field, 0)) {
        CLR_BIT(flagBits.modeBits, taskTable[old_task_id].task_id);
        modeSelect();
    }
    if (IS_BIT_SET(field, 1)) {
        // printMsg("Task ID: %d has been logged\n", currTask.task_id);
    }

    // currTask.cleanPtr();
    taskTable[old_task_id].cleanPtr();
}

void rollback(void) {
    asm volatile (
        "LDR R0, =main_stack_frame\n"
        "LDMIA R0!, {R4-R11}\n"      // Restore main function context
        "LDMIA R0, {R0-R3, R12, LR}\n"
        "MOV SP, R0\n"
    );
    asm volatile (
            "BX LR\n"
    );
}

/**
 * @brief Main scheduling logic
 *
 * Contains the timeslicing between system and mode time,
 * as well as manages the kill/preemption of modes.
 *
 * @param signal timer signal for blocking
 * @param toModeSelect jump buffer in case of preemption
 * @see sysTickHandler()
 * @todo Design check, hardware integration switch jumps
 */
void scheduler() {
    // isSignalBlocked guard not required if blocking signals during system time
    // if (isSignalBlocked(signal)) {
    //     return;
    // }

    scheduler_counter++;
    task_counter++;

    if (!(scheduler_counter % SYSTICK_DUR_M)) {
        //block_signal();
        
        // Store current task_id
        int old_task_id = currTask.task_id;
        
        // Update flags and reselect task
        systemsCheck();
        modeSelect();

        // If same task is selected, resume execution
        if (old_task_id == currTask.task_id) {
            return;
        } else {
            //Preempt
            //printMsg("Task %d preempted by %d\n", old_task_id, currTask.task_id);
            cleanup_handler(0b00, old_task_id);

            // Jmp using flagBits
            //unblock_signal();
//            siglongjmp(*toModeSelect, 1); // PROTOTYPE

             asm volatile(
				"CPSID i\n"                   // Disable interrupts
				"PUSH {R4-R11}\n"             // Save the context of current function
				"LDR R0, =main_stack_frame\n"
				"STMIA R0!, {R4-R11}\n"       // Save the context of main function
				"MRS R0, MSP\n"
				"STMIA R0, {R0-R3, R12, LR}\n"
				"BL rollback\n"
             );
             asm volatile (
            		 "CPSIE i\n"
            		 );
             longjmp(to_mode_select, 1); // SWITCH FOR HARDWARE INTEGRATION
        }
        scheduler_counter = 0;

        //unblock_signal();
    }

    user_timeslice = currTask.taskInterrupts;

    if (task_counter >= user_timeslice) {
        //block_signal();

        task_counter = 0;
        //kill
        cleanup_handler(0b01, currTask.task_id);
        asm volatile(
        				"CPSID i\n"                   // Disable interrupts
        				"PUSH {R4-R11}\n"             // Save the context of current function
        				"LDR R0, =main_stack_frame\n"
        				"STMIA R0!, {R4-R11}\n"       // Save the context of main function
        				"MRS R0, MSP\n"
        				"STMIA R0, {R0-R3, R12, LR}\n"
        				"BL rollback\n"
        				"CPSIE i\n"                   // Enable interrupts
                     );
        longjmp(to_mode_select, 1);

        //unblock_signal();
    }
}
