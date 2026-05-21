#ifndef PID_EXPERIMENT_H
#define PID_EXPERIMENT_H

#include "adcs_math/vector.h"
#include "control/PID/PID.h"
#include "virtual_intellisat.h"

typedef enum PID_experiment{
    PID_EXPERIMENT_SUCCESS,
    PID_EXPERIMENT_ANGVEL_FAILURE,
    PID_EXPERIMENT_MILLIS_FAILURE,
    PID_EXPERIMENT_COMMAND_FAILURE
} PID_status;

/**@brief Performs an experiement for the PID function.
 *
 * @return PID_status A return code.
 */
PID_status PID_experiment(double target, int infinite);


#endif//PID_EXPERIMENT_H