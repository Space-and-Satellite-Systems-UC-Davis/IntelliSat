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
 * task_table - info. about all modes on satellite
 * (mode, timerDur, numInterrupts, & corresponding functions)
 */

// TODO: update ECC mode based on discussions with ELEC
// RS - system, Hamming codes on Idle
struct task_t task_table[] = {
	{LOW_PWR, 6, config_low_pwr, low_pwr, clean_low_pwr, 0},                // Func1 - N/A
	{DETUMBLE, 6, config_detumble, detumble, clean_detumble, 0},            // Func1 - N/A
	{COMMS, 6, config_comms, comms, clean_comms, 0},                        // Func1 - N/A
    {ECC, 6, config_ecc, ecc, clean_ecc, 0},                                // Func1 - N/A
    {EXPERIMENT, 6, config_experiment, experiment, clean_experiment, 0},    // Func1 - Experiment ID (0 for none)
    {IDLE, 6, config_idle, idle, clean_idle, 0}                             // Func1 - N/A
};

volatile struct task_t currtask;

uint32_t user_timeslice;
int scheduler_counter;
u_int32_t task_counter;

jmp_buf to_mode_select;


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
void systems_check() {
    status_check(); // CHARGING flag updated in statusCheck()

    if (low_pwr_time()) {
        SET_BIT(flag_bits.mode_bits, LOW_PWR);
    } else {
        CLR_BIT(flag_bits.mode_bits, LOW_PWR);
    }

    if (detumble_time()) {
        if (IS_BIT_SET(flag_bits.status_bits, COILS)) {
                SET_BIT(flag_bits.mode_bits, DETUMBLE);
        } else {
            // printMsg("Coils circuit is not communicating");
        }
    } else {
	    CLR_BIT(flag_bits.mode_bits, DETUMBLE);
    }

    if (comms_time()) {
        if (IS_BIT_SET(flag_bits.status_bits, ANTENNA)) {
            SET_BIT(flag_bits.mode_bits, COMMS);
        } else {
            // printMsg("Antenna is not deployed\n");
        }
    } else {
        CLR_BIT(flag_bits.mode_bits, COMMS);
    }

    int new_exp_id = experiment_time();
    if (new_exp_id) {
        SET_BIT(flag_bits.mode_bits, EXPERIMENT);
        task_table[4].func_1 = new_exp_id;
    } else {
	    CLR_BIT(flag_bits.mode_bits, EXPERIMENT);
        task_table[4].func_1 = 0;
    }

    if (eccTime())
        SET_BIT(flag_bits.mode_bits, ECC);
    else
	    CLR_BIT(flag_bits.mode_bits, ECC);
    
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
void mode_select() {

    // Counters
    int new_task_id = 0;
    int i;

    // Determine which mode to run next
    for (i = 0; i < 5; i++) {
        if (IS_BIT_SET(flag_bits.mode_bits, i)) {
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
    curr_task = task_table[new_task_id];
    
}

/**
 * @brief Handles common tasks during cleanup
 *
 * Uses a preference bitfield to determine which cleanup
 * actions need to be done including logs and clearing
 * the relevant bit from flag_bits.
 *
 * @param field Preference field
 * @see scheduler()
 * @todo Update to match logging requirements
 */
void cleanup_handler(int8_t field, uint8_t old_task_id) {
    // b0 - flag_bit reset
    // b1 - success log

    if (IS_BIT_SET(field, 0)) {
        CLR_BIT(flag_bits.mode_bits, task_table[old_task_id].task_id);
        mode_select();
    }
    if (IS_BIT_SET(field, 1)) {
        // printMsg("Task ID: %d has been logged\n", currTask.task_id);
    }

    // currTask.cleanPtr();
    task_table[old_task_id].clean_ptr();
}

/**
 * @brief Main scheduling logic
 *
 * Contains the timeslicing between system and mode time,
 * as well as manages the kill/preemption of modes.
 *
 * @see sysTickHandler()
 * @todo Implement
 */
void scheduler() {
    
}
