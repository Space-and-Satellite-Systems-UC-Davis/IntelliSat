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

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief find the angular velocity through change in magnetic vector
 *
 * @param b0 Earth's magnectic field vector (relative to satatlite)
 * @param b1 b0 after delta_t
 * @param delta_t the change in time between mag and mag_prev
 *
 * @return angVel anggular velocity
 */
vec3 findAngVel(vec3 b0, vec3 b1, uint64_t delta_t);

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
 * @brief Compute the magnetic dipole moment (MDM) for 
 *
 * @return true if the delay was performed sucessfully
 */
vec3 computeMDM(vec3 mag_curr, vec3 mag_prev, uint64_t delta_t, vec3 coils_curr,
                vec3 needle);

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