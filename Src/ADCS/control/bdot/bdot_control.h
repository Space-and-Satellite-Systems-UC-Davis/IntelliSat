/**@file bdot_control.h
 * 
 * @brief Interface to the BDOT algorithm.
 *
 * BDOT calculates the optimal direction in which to create a
 *  magnetic dipole in order to slow REALOP during detumbling.
 * The result should have REALOP aligned north-south longways,
 *  spinning on only the longways axis.
 * 
 * @author Jacob Tkeio (jacobtkeio@gmail.com)
 *
 * @date 6/11/2023
 */

#ifndef BDOT_CONTROL
#define BDOT_CONTROL

#include "adcs_math/vector.h"

#include <stdint.h>


/**@brief Calculate a dipole command for all three coils given our
 *  magnetic field and angular velocity data.
 * 
 * @param mag The current magnetic field vector.
 * 		Format: [X, Y, Z] Micro Teslas
 * @param last_mag The previous magnetic field vector.
 * 		Format:    ...
 * @param delta_t Time between the previous and current mag reading.
 * @param coils_current The vec3* that will hold the coil outputs.
 * 		Format: [X, Y, Z] Amperes
 *
 * @return Void.
 */
void bdot_control(
    vec3    mag,
    vec3    last_mag,
    uint64_t  delta_t,
    vec3   *coils_current
);


#endif//BDOT_CONTROL





