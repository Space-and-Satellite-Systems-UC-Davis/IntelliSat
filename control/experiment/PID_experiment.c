#include "control/experiment/PID_experiment.h"
#include "adcs_math/sensors.h"
#include "virtual_intellisat.h"

#include <math.h>
#include <stdint.h>

#define P_GAIN 0.4
#define I_GAIN 0
#define D_GAIN 0.1

PID_status PID_experiment(double target, int infinite)
{

    vi_sensor imu = makeSensor(IMU, ONE, PX);
    vi_sensor hdd = makeSensor(HDD, ONE, PX);

    vec3 angVel_curr = undefined_vec3;
    vec3 angVel_prev = undefined_vec3;

    uint64_t curr_millis = 0;

    // Get current generation for sensor alternation
    int generation = vi_get_experiment_generation();

    // Get IMU sensor choice
    imu.choice = selectSensor(imu, generation);

    // Verify experiment is running

    double throttle = 0;

    // Get the current angular velocity
    if (getIMU(imu, angVel_prev, &angVel_curr))
        return PID_EXPERIMENT_ANGVEL_FAILURE;

    // Get the current time (Virtual Intellisat)
    if (vi_get_curr_millis(&curr_millis))
        return PID_EXPERIMENT_MILLIS_FAILURE;

    // Declare and initlialize PID controller
    PID_controller controller;
    PID_init(target, angVel_curr.z, curr_millis, P_GAIN, I_GAIN, D_GAIN,
             &controller);

    // Run a while loop
    while (infinite || (fabs(target - angVel_curr.z) > 0.1)) {

        if (getIMU(imu, angVel_prev, &angVel_curr))
            return PID_EXPERIMENT_ANGVEL_FAILURE;
        // Get the current time (Virtual Intellisat)
        if (vi_get_curr_millis(&curr_millis))
            return PID_EXPERIMENT_MILLIS_FAILURE;

        // PLug it into the control function
        throttle +=
            PID_command(target, angVel_curr.z, curr_millis, &controller);

        // Clamp the output
        if (throttle >= 0) {
            hdd.choice = ONE;
            if (throttle > 2.5)
                throttle = 2.5;
        } else {
            hdd.choice = TWO;
            if (throttle < -2.5)
                throttle = -2.5;
        }

        // Take output and plug it into HDD
        if (vi_hdd_command(hdd, throttle) == HDD_COMMAND_FAILURE)
            return PID_EXPERIMENT_COMMAND_FAILURE;

        angVel_prev = angVel_curr;
    }

    // Increment generation on successful execution
    vi_increment_experiment_generation();

    return PID_EXPERIMENT_SUCCESS;
}
