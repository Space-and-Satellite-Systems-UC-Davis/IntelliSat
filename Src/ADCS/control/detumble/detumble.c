//
//  detumble.c
//  detumbling
//
//  Created by Chandler Shellabarger on 9/21/24.
//

#include "control/detumble/detumble.h"
#include "adcs_math/calibration.h"
#include "adcs_math/sensors.h"
#include "adcs_math/vector.h"
#include "control/detumble/bdot_control.h"
#include "control/detumble/detumble_util.h"
#include "virtual_intellisat.h"
#include "virtual_loggers.h"
#include "virtual_ros.h"

#include <math.h>

detumble_status detumble(vec3 needle, bool isTesting, uint64_t maxTime,
                         uint64_t minTime)
{
    // Magnotometer & IMU readings
    vec3 mag_curr = undefined_vec3, mag_prev = undefined_vec3;
    vec3 imu_curr = undefined_vec3, imu_prev = undefined_vec3;

    // Time varibles
    uint64_t startTime = 0, curr_millis = 0, prev_millis = 0;
    uint64_t delta_t = 0, timeElapsed = 0;

    // Extra varibles
    vec3 mdm;                   // Magnetic Dipole Moment
    vec3 angVel;                // Angular Velocity
    bool keepDetumbling = true; // Keep loop running?
    int generation = vi_get_detumbling_generation();

    // Declare the sensors
    vi_sensor magnetometer = makeSensor(MAG, ONE, PX);
    magnetometer.choice = selectSensor(magnetometer, generation);

    vi_sensor imu = makeSensor(IMU, ONE, PX);
    imu.choice = selectSensor(imu, generation);

    // Open log file
    char *filename = prependTimeStamp("detumbling.csv");
    createFile(filename, detumbleLOG);
    LOG_FILE *file = openFile(filename);
    logHeader(file);
    detumbleLOGdata data;

    // Get startTime
    vi_get_curr_millis(&curr_millis);
    startTime = curr_millis;

    do {
        vi_enter_critical();

        if (vi_task_has_restarted()) {
            // Return to Schedulers to restart Detumbling
            closeFile(file, "DETUMBLING_HAS_RESTARTED");
            return DETUMBLING_HAS_RESTARTED;
        }

        // Perform delay for the coil magnetic field decay
        if (vi_control_coil(0, 0, 0)) {
            closeFile(file, "DETUMBLING_FAILURE_CONTROL_COILS");
            return DETUMBLING_FAILURE_CONTROL_COILS;
        }

        if (detumbleDelay()) {
            closeFile(file, "DETUMBLING_FAILURE_DELAY_MS");
            return DETUMBLING_FAILURE_DELAY_MS;
        }

        // Get MAG readings
        mag_prev = mag_curr;
        getMag(magnetometer, mag_prev, &mag_curr);

        // Compute the magetic dipole moment: M = -k(bDot - n)
        mdm = computeMDM(magnetometer, mag_curr, mag_prev, delta_t, needle);

        // Send control command to coils
        if (vi_control_coil(mdm.x, mdm.y, mdm.z)) {
            closeFile(file, "DETUMBLING_FAILURE_CONTROL_COILS");
            return DETUMBLING_FAILURE_CONTROL_COILS;
        }

        // Get IMU readings (A.K.A.: Angular Velocity) for exit condition
        imu_prev = imu_curr;
        getIMU(imu, imu_prev, &imu_curr);

        // Keep track of deltaT and timeElapsed
        prev_millis = curr_millis;
        vi_get_curr_millis(&curr_millis);
        delta_t = get_delta_t(curr_millis, prev_millis);

        // Decide whether detumbling needs to continue
        timeElapsed = get_delta_t(curr_millis, startTime);
        bool isTimeOut = timeElapsed > maxTime;
        bool isTooSoon = timeElapsed < minTime;
        bool isTooFast = aboveThreshold(imu_curr, imu_prev, 0.5);

        keepDetumbling = isTooSoon || (!isTimeOut && isTooFast);

        // Updating the data struct with the new values for logging
        data.imu = imu_curr;
        data.mag = mag_curr;
        data.mdm = mdm;

        logRecord(file, &data);
        vi_exit_critical();

    } while (isTesting || keepDetumbling);

    // Increment generation on successful execution
    vi_increment_detumbling_generation();

    closeFile(file, "Completed");

    return DETUMBLING_SUCCESS;
}

// Hello world Happy New Year 2026 :) Yippee