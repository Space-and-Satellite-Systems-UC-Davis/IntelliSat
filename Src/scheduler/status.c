/**
 * @file status.c
 * @brief Status bit modifications
 *
 * Handles all status bit related
 * updates.
 *
 * @authors Nithin Senthil, Parteek Singh, Jacob Tkeo
 * @date 9/8/23
 */

#include "status.h"

/**
 * @brief Checks the status of peripheral systems
 *
 * Checks with required peripherals for 
 * functionality and updates status bits.
 *
 * @see systemsCheck()
 * @note Implemented by relevant subteams
 * @todo Separate into wrapper functions for implementation
 */
void statusCheck() {
    //printMsg("Running status check\n");
    // Sets all status flags for testing (ASSUMES ALL SYSTEMS FUNCTIONAL)
    for(int i = 0; i < 5; i++) {
        SET_BIT(flagBits.statusBits, i);
    }
}
