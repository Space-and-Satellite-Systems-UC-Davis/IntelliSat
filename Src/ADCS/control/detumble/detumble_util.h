/**
 * @file detumble_util.h
 *
 * @brief Interface between sensors and control functions
 *
 * @author Chunho Li (lchli@ucdavis.edu) 6/23/2025
 */

#ifndef DETUMBLE_UTIL_H
#define DETUMBLE_UTIL_H

#include "adcs_math/vector.h"
#include "control/detumble/detumble.h"
#include "virtual_intellisat.h"
#include "math.h"

#include <stdbool.h>
#include <stdint.h>



/**
 * @brief convert the coils current into magnetic field magitude
 *
 * @param current current coils current
 *
 * @return magnetic field magitude
 */
double computeB_coils(double current);

/**
 * @brief compute the time needed for the coil's magnetic field to decay
 * 
 * @return 
 */
double computeDecay(double B_initial);

/**
 * @brief Perform the delay for the magnetic coils 
 *
 * @return true if the delay was performed sucessfully
 */
bool detumbleDelay(vec3 mdm);

/**
 * @brief checks if angular velocity exceeds threshold
 *
 * @param input angular velocity vector to be examined
 * @param threshold should be self explanatory
 *
 * @return true if threshold has been exceeded
 */
bool aboveThreshold(vec3 input, double threshold);

/**
 * @brief checks if current exceeds threshold
 *
 * @param mdm magnetic dipole moment
 *
 * @return mdm but capped at 0.158f
 */
void capCurrent(vec3 *mdm);

#endif