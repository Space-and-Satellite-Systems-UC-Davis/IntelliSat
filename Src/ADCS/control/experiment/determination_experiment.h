#ifndef DETERMINATION_EXPERIMENT_H
#define DETERMINATION_EXPERIMENT_H

#include "adcs_math/vector.h"
#include "control/PID/PID.h"
#include "virtual_intellisat.h"


typedef enum determination_experiment{
    DETERMINATION_EXPERIMENT_SUCCESS,
    DETERMINATION_EXPERIMENT_MILLIS_FAILURE,
    DETERMINATION_EXPERIMENT_HDD_COMMAND_FAILURE
}determination_exp_status;

/*
 * @brief Performs an experiement using the function.
 *        Uses determination to calculate the angular velocity 
 *        instead of the IMU
 *
 * @return PID_status A return code.
 */
determination_exp_status determination_experiment();


#endif//DETERMINATION_EXPERIMENT_H
