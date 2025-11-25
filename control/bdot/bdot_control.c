/**@file bdot_control.c
 * 
 * @brief Implementation of the BDOT algorithm.
 * 
 * This file was transcribed from the python version on github.
 *  See github tag 'python-archive' for the original.
 * BDOT should only use magnetometer and time readings.
 *
 * @author Jacob Tkeio (jacobtkeio@gmail.com)
 *
 * @date 6/11/2023
 */

#include "bdot_control.h"
#include "adcs_math/vector.h"

void bdot_control(
    vec3    mag,
    vec3    last_mag,
    uint64_t  delta_t,
    vec3   *bdot
) {
    vec3 derivative;    // = (mag - last_mag)/delta_t
    
    if (delta_t == 0) {
        *bdot = (vec3){0.0,0.0,0.0};
        return;
    }

    vec_scalar(-1.0, last_mag, &last_mag);
    vec_add(mag, last_mag, &derivative);
    vec_scalar(1.0/delta_t, derivative, &derivative);

	(*bdot) = derivative;
}





