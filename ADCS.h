/**@file ADCS.h
 *
 * @brief Intellisat's interface to the ADCS software.
 *
 * @author Jacob Tkeio (jacobtkeio@gmail.com)
 *
 * @date 7/30/2023
 */

#ifndef ADCS_H
#define ADCS_H

#include "adcs_math/matrix.h"

typedef enum {
    ADCS_DETUMBLE,
    ADCS_INITIAL_DETUMBLE,
    ADCS_HDD_EXP_ANGVEL,
    ADCS_HDD_EXP_TRIAD,
    ADCS_HDD_EXP_RAMP,
    ADCS_TESTING,
    ADCS_COILS_TESTING,
    ADCS_HDD_TESTING,
    ADCS_ROTISSERIE
} adcs_mode; // TODO: full list

typedef enum {
    ADCS_MAIN_SUCCESS,
    ADCS_MAIN_UNKNOWN,
    ADCS_MAIN_ECLIPSE,
    ADCS_MAIN_HDD_ERR,
    ADCS_MAIN_IMU_ERR,
    ADCS_MAIN_DETUMBLE_ERR,
    ADCS_MAIN_COILS_TESTING_ERR,
    ADCS_ROTISSERIE_ERR
} adcs_main_status; // TODO: full list

typedef enum{
	GET_ATTITUDE_SUCCESS,
    GET_ATTITUDE_NO_TLE,
    GET_ATTITUDE_POS_LOOKUP_ERROR,
    GET_ATTITUDE_IGRF_TIME_ERROR,
    GET_ATTITUDE_TRIAD_ERROR,
    GET_ATTITUDE_EPOCH_FAILURE,
    GET_ATTITUDE_MAG_FAILURE,
    GET_ATTITUDE_CSS_FAILURE
} adcs_get_attitude_status;

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

/**@brief Command ADCS to run a mode.
 *
 * Blocks during mode execution.
 *
 * @param mode The mode to run.
 *
 * @return The mode's status code.
 */
adcs_main_status ADCS_MAIN(adcs_mode mode);

/**@brief Ask ADCS to recommend a mode to run.
 *
 * @return The recommended mode.
 */
adcs_mode ADCS_recommend_mode();

/**
 * @brief Checks to see if currently in eclipse
 *
 * @return 0 or 1 depending on if it is not or is in eclipse respectively
 */
int ADCS_is_in_eclipse();

/**
 * @brief Get the current attitude
 *
 * @param attitude Return-by-reference ptr to attitude matrix
 *
 * @return Status code
 */
adcs_get_attitude_status ADCS_get_attitude(mat3 *attitude);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // ADCS_H
