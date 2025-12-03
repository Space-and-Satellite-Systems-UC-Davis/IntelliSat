//
//  detumble.c
//  detumbling
//
//  Created by Chandler Shellabarger on 9/21/24.
//

#define LIMIT 100

#include "detumble.h"
#include "adcs_math/calibration.h"
#include "adcs_math/sensors.h"
#include "adcs_math/vector.h"
#include "control/bdot/bdot_control.h"
#include "detumble_util.h"
#include "virtual_intellisat.h"

#include <math.h>

detumble_status detumble(vec3 needle, bool isTesting) {
    vec3 mag_curr, mag_prev = {0,0,0}; // Magnotometer readings
    //vec3 coils_curr;
    uint64_t startTime = 0, curr_millis = 0, prev_millis = 0, delta_t = 0;
    vec3 mdm;            // Magnetic Dipole Moment
    vec3 angVel;         // Angular Velocity
    bool keepDetumbling; // Keep loop running?
    int generation = vi_get_detumbling_generation();

    // Declare varibles for sensor alternation
    vi_sensor magnotometer; // Initialize it to VI_MAG1
    magnotometer.component = MAG;
    magnotometer.choice =
        sensor_pair_choice(magnotometer, generation) == 1 ? ONE : TWO;

    if (vi_get_curr_millis(&curr_millis))
        return DETUMBLING_FAILURE_CURR_MILLIS;
    startTime = curr_millis;

    // Compute Angular Velocity
    if (getMag(magnotometer, mag_prev, &mag_curr))
        return DETUMBLING_FAILURE_MAGNOTOMETER;

    angVel = findAngVel(mag_prev, mag_curr, delta_t);

    // Note: May be do something to account for integer overflow
    do {
        prev_millis = curr_millis;
        if (vi_get_curr_millis(&curr_millis))
            return DETUMBLING_FAILURE_CURR_MILLIS;

        // Perform delay for the coil magnetic field decay
        if (vi_control_coil(0, 0, 0))
            return DETUMBLING_FAILURE_CONTORL_COILS;
        if (detumbleDelay(mdm))
            return DETUMBLING_FAILURE_DELAY_MS;

        delta_t = get_delta_t(curr_millis, prev_millis);

        mag_prev = mag_curr;
        if (getMag(magnotometer, mag_prev, &mag_curr))
            return DETUMBLING_FAILURE_MAGNOTOMETER;

        // Compute the magetic dipole moment: M = -k(bDot - n)
        mdm = computeMDM(mag_curr, mag_prev, delta_t, needle);

        // Send control command to coils
        if (vi_control_coil(mdm.x, mdm.y, mdm.z))
            return DETUMBLING_FAILURE_CONTORL_COILS;

        angVel = findAngVel(mag_prev, mag_curr, delta_t);

        // Decide whether detumbling needs to continue
        bool isTimeOut = curr_millis - startTime > LIMIT;
        keepDetumbling = aboveThreshold(angVel, 0.5) && isTimeOut;

    } while (isTesting || keepDetumbling);

    // Increment generation on successful execution
    vi_increment_detumbling_generation();

    return DETUMBLING_SUCCESS;
}
