/**
 * @file ADCS.c
 *
 * @brief Implementation of Intellisat's interface to the ADCS software.
 *
 * @author Jacob Tkeio (jacobtkeio@gmail.com)
 * @date 8/5/2023
 */

#include "ADCS.h"
#include "adcs_math/sensors.h"
#include "adcs_math/vector.h"
#include "control/detumble/detumble.h"
#include "control/experiment/PID_experiment.h"
#include "control/experiment/determination_experiment.h"
#include "control/experiment/ramp_experiment.h"
#include "adcs_math/sensors.h"
#include "determination/determination.h"
#include "virtual_intellisat.h"

#include <stdbool.h>

adcs_main_status detumbleEX(vec3 needle, uint64_t minTime, uint64_t maxTime)
{
    switch (detumble(needle, false, minTime, maxTime)) {
        case DETUMBLING_SUCCESS:
            return ADCS_MAIN_SUCCESS;
        case DETUMBLING_FAILURE_CURR_MILLIS:
        case DETUMBLING_FAILURE_MAGNETOMETER:
        case DETUMBLING_FAILURE_IMU:
        case DETUMBLING_FAILURE_CONTROL_COILS:
        case DETUMBLING_FAILURE_DELAY_MS:
        case DETUMBLING_HAS_RESTARTED:
            return ADCS_MAIN_DETUMBLE_ERR;
    }
}

adcs_main_status ADCS_MAIN(adcs_mode mode)
{
    switch (mode) {
        case ADCS_INITIAL_DETUMBLE:
            return detumbleEX((vec3){ 0, 0, 0 }, 3600, 36000);

        case ADCS_DETUMBLE:
            return detumbleEX((vec3){ 0, 0, 0 }, 0, 3600);

        // SHOULD NEVER EVER EVER BE ENGAGED IN PRODUCTION (SPACE)
        case ADCS_COILS_TESTING:
            switch (detumble((vec3){ 0, 0, 0 }, true, 0, 3600)) {
                case DETUMBLING_SUCCESS:
                    break;
                case DETUMBLING_FAILURE_CURR_MILLIS:
                    // Do something
                case DETUMBLING_FAILURE_MAGNETOMETER:
                    // Do something
                case DETUMBLING_FAILURE_IMU:
                    // Do something
                case DETUMBLING_FAILURE_CONTROL_COILS:
                    // Do something
                case DETUMBLING_FAILURE_DELAY_MS:
                    // Do something
                case DETUMBLING_HAS_RESTARTED:
                    return ADCS_MAIN_COILS_TESTING_ERR;
            }
            break;

        case ADCS_HDD_EXP_ANGVEL:
            PID_experiment(0, 0);
            break;

        case ADCS_HDD_EXP_TRIAD:
            determination_experiment();
            break;

        case ADCS_HDD_EXP_RAMP:
            ramp_experiment();
            break;

        case ADCS_HDD_TESTING:
            PID_experiment(0, 1); // for infinite
            break;

        case ADCS_TESTING:
            vi_print("Testing!");
            break;

        case ADCS_ROTISSERIE:
            switch (PID_experiment(.0872665, 1)) {
                case PID_EXPERIMENT_SUCCESS:
                    break;
                case PID_EXPERIMENT_ANGVEL_FAILURE:
                case PID_EXPERIMENT_MILLIS_FAILURE:
                case PID_EXPERIMENT_COMMAND_FAILURE:
                    return ADCS_ROTISSERIE_ERR;
            }

            return ADCS_MAIN_SUCCESS;
    }
}

adcs_mode ADCS_recommend_mode()
{
    int iteration = vi_get_experiment_generation(); // Starts as true on
                                                    // reboot
    vi_increment_experiment_generation();
    adcs_mode mode;

    if (iteration == 0) {
        mode = ADCS_HDD_EXP_ANGVEL;
        // loop through other experiments
    } else if (iteration % 3 == 1) {
        mode = ADCS_HDD_EXP_TRIAD;
    } else if (iteration % 3 == 2) {
        mode = ADCS_HDD_EXP_RAMP;
    } else /*if (iteration % 3 == 0)*/ {
        mode = ADCS_HDD_EXP_ANGVEL;
    }

    return mode;
}

int ADCS_is_in_eclipse() { return is_in_eclipse(); }

adcs_get_attitude_status ADCS_get_attitude(mat3 *attitude)
{
    determination_status det_status = determination(attitude);
    switch (det_status) {
        case DET_SUCCESS:
            return GET_ATTITUDE_SUCCESS;
        case DET_NO_TLE:
            return GET_ATTITUDE_NO_TLE;
        case DET_POS_LOOKUP_ERROR:
            return GET_ATTITUDE_POS_LOOKUP_ERROR;
        case DET_IGRF_TIME_ERROR:
            return GET_ATTITUDE_IGRF_TIME_ERROR;
        case DET_TRIAD_ERROR:
            return GET_ATTITUDE_TRIAD_ERROR;
        case DET_EPOCH_FAILURE:
            return GET_ATTITUDE_EPOCH_FAILURE;
        case DET_MAG_FAILURE:
            return GET_ATTITUDE_MAG_FAILURE;
        case DET_CSS_FAILURE:
            return GET_ATTITUDE_CSS_FAILURE;
    }
}
