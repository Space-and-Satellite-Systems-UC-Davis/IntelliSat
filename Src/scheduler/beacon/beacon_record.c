#include "beacon_record.h"

#include <string.h>

void build_beacon_record(log_record_idle *out,
                          const beacon_sensor_snapshot_t *snap,
                          const beacon_battery_window_t *batt_window) {
    memset(out, 0, sizeof(*out));

    // TODO: no boot counter exists anywhere in the codebase yet.
    out->boot_number = 0;

    out->current_year = snap->year;
    out->current_month = snap->month;
    out->current_date = snap->date;
    out->current_hour = snap->hour;
    out->current_minute = snap->minute;
    out->current_second = snap->second;

    // TODO: no action-tracking mechanism exists yet.
    out->last_action_taken = 0;

    // TODO: no uplink-timestamp storage exists yet.
    out->last_uplink_year = 0;
    out->last_uplink_month = 0;
    out->last_uplink_date = 0;
    out->last_uplink_hour = 0;
    out->last_uplink_minute = 0;
    out->last_uplink_second = 0;

    // TODO: no experiment-count tracking exists yet.
    out->num_new_experiments = 0;

    out->batt_level = snap->batt_voltage;
    out->batt_7_day_low = batt_window->batt_low;
    out->batt_7_day_avg = batt_window->batt_avg;
    out->batt_7_day_high = batt_window->batt_high;

    out->imu_angvel_x = snap->imu_gx;
    out->imu_angvel_y = snap->imu_gy;
    out->imu_angvel_z = snap->imu_gz;

    out->mag_x = (float) snap->mag_x;
    out->mag_y = (float) snap->mag_y;
    out->mag_z = (float) snap->mag_z;

    out->css_px0 = snap->css_px0;
    out->css_px1 = snap->css_px1;
    out->css_py0 = snap->css_py0;
    out->css_py1 = snap->css_py1;
    out->css_pz0 = snap->css_pz0;
    out->css_pz1 = snap->css_pz1;
    out->css_nx0 = snap->css_nx0;
    out->css_nx1 = snap->css_nx1;
    out->css_ny0 = snap->css_ny0;
    out->css_ny1 = snap->css_ny1;
    out->css_nz0 = snap->css_nz0;
    out->css_nz1 = snap->css_nz1;

    out->temp_px = snap->temp_px;
    out->temp_nx = snap->temp_nx;
    out->temp_py = snap->temp_py;
    out->temp_ny = snap->temp_ny;

    ADCS_get_n_error(ADCS_ERROR_LOG_SIZE, out->error_msg);

    // ADCS_get_attitude() can fail; only trust the attitude output on
    // success, otherwise leave it zeroed (memset above) rather than
    // publish stale or garbage data.
    if (snap->det_status == GET_ATTITUDE_SUCCESS) {
        out->attitude = snap->attitude;
    }

    out->is_in_eclipse = snap->is_in_eclipse ? 1 : 0;
}
