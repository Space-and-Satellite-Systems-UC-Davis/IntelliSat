#include "ramp_experiment.h"
#include "adcs_math/sensors.h"
#include "virtual_intellisat.h"

//TODO: HDD alternation?
#define HDD_CHOICE VI_HDD1


run_ramp_experiment_status ramp_experiment(){

    ramp_controller controller;
    uint64_t t0;

    vi_sensor Hdd = {HDD, ONE, PX};
    vi_hdd_command_status command_status;
    vi_get_curr_millis_status millis_status = vi_get_curr_millis(&t0);
    if (millis_status == GET_CURR_MILLIS_FAILURE){
        return RUN_RAMP_EXPERIMENT_MILLIS_FAILURE;
    }

    init_ramp_controller(t0, 1000, 1000, 2000, 1.0, &controller);
    uint64_t ti = t0;

    while(get_delta_t(ti, t0) < controller.left_leg_length_ms 
    + controller. right_leg_length_ms 
    + controller.plateau_length_ms){

        millis_status = vi_get_curr_millis(&ti);
        if (millis_status == GET_CURR_MILLIS_FAILURE){
            return RUN_RAMP_EXPERIMENT_MILLIS_FAILURE;
        }

        double command = linear_ramp_command(ti , &controller);
        
        command_status = vi_hdd_command(Hdd, command);
        if (command_status == HDD_COMMAND_FAILURE){
            return RUN_RAMP_EXPERIMENT_COMMAND_FAILURE;
        }
    }
    
    return RUN_RAMP_EXPERIMENT_SUCCESS;
}
