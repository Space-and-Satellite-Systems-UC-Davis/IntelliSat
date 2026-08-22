#ifndef SCHEDULER_BEACON_BEACON_RECORD_H_
#define SCHEDULER_BEACON_BEACON_RECORD_H_

#include <stdint.h>

#include "logging_records.h"
#include "adcs_math/matrix.h"
#include "ADCS.h"

#include "beacon_stats.h"

/**
 * Raw sensor values for one beacon cycle, gathered by beacon_task.c.
 */
typedef struct {
    uint8_t boot_number;
    float batt_voltage;

    float imu_gx;
    float imu_gy;
    float imu_gz;

    int16_t mag_x;
    int16_t mag_y;
    int16_t mag_z;

    uint8_t css_px0;
    uint8_t css_px1;
    uint8_t css_nx0;
    uint8_t css_nx1;
    uint8_t css_py0;
    uint8_t css_py1;
    uint8_t css_ny0;
    uint8_t css_ny1;
    uint8_t css_pz0;
    uint8_t css_pz1;
    uint8_t css_nz0;
    uint8_t css_nz1;

    uint8_t temp_px;
    uint8_t temp_nx;
    uint8_t temp_py;
    uint8_t temp_ny;

    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t year;
    uint8_t month;
    uint8_t date;
    
    uint8_t last_action_taken;

    uint8_t num_new_experiments;

    mat3 attitude;
    adcs_get_attitude_status det_status;

    int is_in_eclipse;
} beacon_sensor_snapshot_t;

/**
 * Builds a log_record_idle from a sensor snapshot and the current
 * rolling-window battery stats.
 */
void build_beacon_record(log_record_idle *out,
                          const beacon_sensor_snapshot_t *snap,
                          const beacon_battery_window_t *batt_window);

#endif /* SCHEDULER_BEACON_BEACON_RECORD_H_ */
