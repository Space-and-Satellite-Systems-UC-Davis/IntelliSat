#ifndef RAMP_EXPERIMENT_H
#define RAMP_EXPERIMENT_H

#include "adcs_math/vector.h"
#include "control/ramp/ramp.h"
#include "virtual_intellisat.h"

typedef enum {
    RUN_RAMP_EXPERIMENT_SUCCESS,
    RUN_RAMP_EXPERIMENT_MILLIS_FAILURE,
    RUN_RAMP_EXPERIMENT_COMMAND_FAILURE,
    RUN_RAMP_EXPERIMENT_HAS_RESTARTED
} run_ramp_experiment_status;

/**
 * @brief Performs an experiement for the HDD to ramp .
 *
 * @return PID_status A return code.
 */
run_ramp_experiment_status ramp_experiment();

#endif
