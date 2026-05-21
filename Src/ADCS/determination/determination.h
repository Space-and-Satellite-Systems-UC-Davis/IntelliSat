/**
 * @file determination/determination.h
 *
 * @brief Interface to attitude determination routine.
 *
 * @author Jacob Tkeio (jacobtkeio@gmail.com)
 *
 * @date 12/2/2023
 */

#ifndef DETERMINATION_H
#define DETERMINATION_H

#include "adcs_math/vector.h"
#include "adcs_math/matrix.h"


typedef enum {
    DET_SUCCESS,
    DET_NO_TLE,
    DET_POS_LOOKUP_ERROR,
    DET_IGRF_TIME_ERROR,
    DET_TRIAD_ERROR,
    DET_EPOCH_FAILURE,
    DET_MAG_FAILURE,
    DET_CSS_FAILURE
} determination_status;


/**
 * @brief Calculate attitude from time and sun+mag vectors.
 *
 * Will run SGP4, SPA, and IGRF.
 *
 * @param year,month,day,hour,minute,second The current time.
 * @param measured_mag The measured magnetic field vector.
 * @param measured_sun The measured sun vector.
 * @param attitude Return-by-reference ptr to attitude matrix.
 *
 * @return Status code.
 */
determination_status
determination(
    mat3 *attitude
);

void get_earth_direction(
    vec3* earth_attitude);

#endif//DETERMINATION_H
