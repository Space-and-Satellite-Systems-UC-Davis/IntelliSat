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


const double control_constant = 67200.0; // TODO: tune :p

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

vec3 findAngVel(vec3 b0, vec3 b1, uint64_t delta_t) {

    vec3 bdot;   // The velocity vector pointing from b0 to b1
    vec3 angVel; // The angular velocity

    if (delta_t == 0)
        return (vec3){0.0, 0.0, 0.0};

    // angVel = 1/|b0| x bDot
    bdot_control(b1, b0, delta_t, &bdot);
    vec_scalar((1.0 / vec_mag(b0)), bdot, &angVel);

    // Convert into degree per second
    double RPSmultiplier = (180 / M_PI) * 1000;
    vec_scalar(RPSmultiplier, angVel, &angVel);

    return angVel;
}

/* MDM = - K * ( Bdot * needle) */
vec3 computeMDM(vec3 mag_curr, vec3 mag_prev, uint64_t delta_t, vec3 needle) {

    vec3 mdm;
    vec3 coils_curr;
    bdot_control(mag_curr, mag_prev, delta_t, &coils_curr);
    vec_sub(coils_curr, needle, &coils_curr);
    vec_scalar(-control_constant, coils_curr, &mdm);
    capCurrent(&mdm);

    return mdm;
}




