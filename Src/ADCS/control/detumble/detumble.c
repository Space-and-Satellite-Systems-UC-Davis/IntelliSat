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
#include "virtual_rtos.h"

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

    // Get startTime
    if (vi_get_curr_millis(&curr_millis))
        return DETUMBLING_FAILURE_CURR_MILLIS;
    startTime = curr_millis;

    // Open log file
    LOG_FILE *file =
        createFile(prependTimeStamp("detumbling.csv"), detumbleLOG);
    detumbleLOGdata data;

    detumble_status status = DETUMBLING_SUCCESS;
    char *exit_reason = "Completed";

    do {
        vi_enter_critical();

        if (vi_task_has_restarted()) {
            // Return to Schedulers to restart Detumbling
            status = DETUMBLING_HAS_RESTARTED;
            exit_reason = "DETUMBLING_HAS_RESTARTED";
            vi_exit_critical();
            break;
        }

        // Perform delay for the coil magnetic field decay
        if (vi_control_coil(0, 0, 0)) {
            status = DETUMBLING_FAILURE_CONTROL_COILS;
            exit_reason = "DETUMBLING_FAILURE_CONTROL_COILS";
            vi_exit_critical();
            break;
        }

        if (detumbleDelay()) {
            status = DETUMBLING_FAILURE_DELAY_MS;
            exit_reason = "DETUMBLING_FAILURE_DELAY_MS";
            vi_exit_critical();
            break;
        }

        // Get MAG readings
        mag_prev = mag_curr;
        if (getMag(magnetometer, mag_prev, &mag_curr)) {
            status = DETUMBLING_FAILURE_MAGNETOMETER;
            exit_reason = "DETUMBLING_FAILURE_MAGNETOMETER";
            vi_exit_critical();
            break;
        }

        // Compute the magetic dipole moment: M = -k(bDot - n)
        mdm = computeMDM(magnetometer, mag_curr, mag_prev, delta_t, needle);

        // Send control command to coils
        if (vi_control_coil(mdm.x, mdm.y, mdm.z)) {
            status = DETUMBLING_FAILURE_CONTROL_COILS;
            exit_reason = "DETUMBLING_FAILURE_CONTROL_COILS";
            vi_exit_critical();
            break;
        }

        // Get IMU readings (A.K.A.: Angular Velocity) for exit condition
        imu_prev = imu_curr;
        if (getIMU(imu, imu_prev, &imu_curr)) {
            status = DETUMBLING_FAILURE_IMU;
            exit_reason = "DETUMBLING_FAILURE_IMU";
            vi_exit_critical();
            break;
        }

        // Keep track of deltaT and timeElapsed
        prev_millis = curr_millis;
        if (vi_get_curr_millis(&curr_millis)) {
            status = DETUMBLING_FAILURE_CURR_MILLIS;
            exit_reason = "DETUMBLING_FAILURE_CURR_MILLIS";
            vi_exit_critical();
            break;
        }
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

        vi_exit_critical();

        // Logged outside the critical section on purpose: when littlefs's cache
        // fills, logRecord() writes through to the QSPI FLASH, and a block
        // boundary means an erase. That is far too long to sit with interrupts
        // disabled.
        logRecord(file, &data);

        // In testing mode the loop is deliberately endless -- it only leaves
        // via a restart notification or a failure above, so DETUMBLING_SUCCESS
        // is unreachable and closeFile() always reports why it stopped.
    } while (isTesting || keepDetumbling);

    // Increment generation on successful execution
    if (status == DETUMBLING_SUCCESS)
        vi_increment_detumbling_generation();

    closeFile(file, exit_reason);

    return status;
}

// Hello world Happy New Year 2026 :) Yippee