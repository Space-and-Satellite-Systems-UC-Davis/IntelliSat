#include "control/experiment/determination_experiment.h"
#include "adcs_math/sensors.h"
#include "adcs_math/vector.h"
#include "determination/determination.h"

// TODO: HDD alternation? (Update: rn it's defaulted to VI_HDD1)

determination_exp_status determination_experiment() {
  mat3 prevAttitude;
  mat3 currAttitude;
  vec3 sun;
  vi_sensor hdd = {HDD, ONE, PX};

  // Get current generation for sensor alternation
  //int generation = vi_get_experiment_generation();

  // TODO: default values for now, waiting for sun sensors to implement get_sun
  sun.x = 0;
  sun.y = 0;
  sun.z = 0;

  determination(&prevAttitude);

  // TODO: generalizing initial angular velocity as 0; might have to fix
  int angvel_z = 0;

  // Get the current time (Virtual Intellisat)
  uint64_t prev_millis = 0;
  uint64_t curr_millis = 0;
  if (vi_get_curr_millis(&prev_millis) == GET_CURR_MILLIS_FAILURE)
    return DETERMINATION_EXPERIMENT_MILLIS_FAILURE;

  // Declare and initlialize PID controller
  PID_controller controller;
  double target = 0;
  PID_init(target, angvel_z, prev_millis, 1, 1, 1, &controller);

  // Run a while loop
  while (fabs(target - angvel_z) > 0.1) {
    vi_delay_ms(100);

    // default values for now, waiting for sun sensors to implement get_sun
    sun.x = 0;
    sun.y = 0;
    sun.z = 0;


    determination(&currAttitude);
    // Get the current time (Virtual Intellisat)
    if (vi_get_curr_millis(&curr_millis) == GET_CURR_MILLIS_FAILURE)
      return DETERMINATION_EXPERIMENT_MILLIS_FAILURE;

    int delta_t = get_delta_t(curr_millis, prev_millis);

    mat3 derivative;
    mat_sub(currAttitude, prevAttitude, &derivative);
    mat_scalar(1.0 / delta_t, derivative, &derivative);
    double zrotation = derivative.y1;

    // PLug it into the control function
    double throttle = PID_command(target, zrotation, curr_millis, &controller);
    
    // Take output and plug it into HDD
    if (vi_hdd_command(hdd, throttle) == HDD_COMMAND_FAILURE)
      return DETERMINATION_EXPERIMENT_HDD_COMMAND_FAILURE;
    prevAttitude = currAttitude;
    prev_millis = curr_millis;
  }

  // Increment generation on successful execution
  vi_increment_experiment_generation();

  return DETERMINATION_EXPERIMENT_SUCCESS;
}
