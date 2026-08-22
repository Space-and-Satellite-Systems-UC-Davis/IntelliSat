#include "framework.h"

#include "beacon_record.h"

static beacon_sensor_snapshot_t make_snapshot(void) {
    beacon_sensor_snapshot_t snap = {0};

    snap.boot_number = 42;

    snap.batt_voltage = 3.75f;

    snap.imu_gx = 1.5f;
    snap.imu_gy = -2.5f;
    snap.imu_gz = 0.25f;

    snap.mag_x = 100;
    snap.mag_y = -200;
    snap.mag_z = 300;

    snap.css_px0 = 10;
    snap.css_px1 = 11;
    snap.css_nx0 = 20;
    snap.css_nx1 = 21;
    snap.css_py0 = 30;
    snap.css_py1 = 31;
    snap.css_ny0 = 40;
    snap.css_ny1 = 41;
    snap.css_pz0 = 50;
    snap.css_pz1 = 51;
    snap.css_nz0 = 60;
    snap.css_nz1 = 61;

    snap.temp_px = 70;
    snap.temp_nx = 71;
    snap.temp_py = 72;
    snap.temp_ny = 73;

    snap.hour = 12;
    snap.minute = 34;
    snap.second = 56;
    snap.year = 26;
    snap.month = 7;
    snap.date = 24;
    snap.last_action_taken = 3;

    snap.attitude = (mat3) {
        .x1 = 1, .x2 = 2, .x3 = 3,
        .y1 = 4, .y2 = 5, .y3 = 6,
        .z1 = 7, .z2 = 8, .z3 = 9
    };
    snap.det_status = GET_ATTITUDE_SUCCESS;

    snap.is_in_eclipse = 1;

    return snap;
}

static beacon_battery_window_t make_batt_window(void) {
    beacon_battery_window_t w;
    w.batt_low = 3.5f;
    w.batt_avg = 3.7f;
    w.batt_high = 3.9f;
    return w;
}

static void test_passthrough_fields(void) {
    beacon_sensor_snapshot_t snap = make_snapshot();
    beacon_battery_window_t window = make_batt_window();
    log_record_idle out;

    build_beacon_record(&out, &snap, &window);

    TEST_ASSERT(out.boot_number == snap.boot_number);

    TEST_ASSERT(out.current_year == snap.year);
    TEST_ASSERT(out.current_month == snap.month);
    TEST_ASSERT(out.current_date == snap.date);
    TEST_ASSERT(out.current_hour == snap.hour);
    TEST_ASSERT(out.current_minute == snap.minute);
    TEST_ASSERT(out.current_second == snap.second);

    TEST_ASSERT(out.last_action_taken == snap.last_action_taken);

    TEST_ASSERT_FLT(out.batt_level, snap.batt_voltage, 1e-6);
    TEST_ASSERT_FLT(out.batt_7_day_low, window.batt_low, 1e-6);
    TEST_ASSERT_FLT(out.batt_7_day_avg, window.batt_avg, 1e-6);
    TEST_ASSERT_FLT(out.batt_7_day_high, window.batt_high, 1e-6);

    TEST_ASSERT_FLT(out.imu_angvel_x, snap.imu_gx, 1e-6);
    TEST_ASSERT_FLT(out.imu_angvel_y, snap.imu_gy, 1e-6);
    TEST_ASSERT_FLT(out.imu_angvel_z, snap.imu_gz, 1e-6);

    TEST_ASSERT_FLT(out.mag_x, (float) snap.mag_x, 1e-6);
    TEST_ASSERT_FLT(out.mag_y, (float) snap.mag_y, 1e-6);
    TEST_ASSERT_FLT(out.mag_z, (float) snap.mag_z, 1e-6);

    TEST_ASSERT(out.is_in_eclipse == 1);
}

static void test_unimplemented_fields_are_zero(void) {
    beacon_sensor_snapshot_t snap = make_snapshot();
    beacon_battery_window_t window = make_batt_window();
    log_record_idle out;

    build_beacon_record(&out, &snap, &window);

    TEST_ASSERT(out.last_uplink_year == 0);
    TEST_ASSERT(out.last_uplink_month == 0);
    TEST_ASSERT(out.last_uplink_date == 0);
    TEST_ASSERT(out.last_uplink_hour == 0);
    TEST_ASSERT(out.last_uplink_minute == 0);
    TEST_ASSERT(out.last_uplink_second == 0);
    TEST_ASSERT(out.num_new_experiments == 0);
}

static void test_error_msg_populated(void) {
    beacon_sensor_snapshot_t snap = make_snapshot();
    beacon_battery_window_t window = make_batt_window();
    log_record_idle out;

    build_beacon_record(&out, &snap, &window);

    for (int i = 0; i < ADCS_ERROR_LOG_SIZE; i++) {
        TEST_ASSERT(out.error_msg[i].error == (uint8_t) i);
        TEST_ASSERT(out.error_msg[i].unix_timestamp == 1000u + (uint32_t) i);
    }
}

static void test_css_temp_passthrough(void) {
    beacon_sensor_snapshot_t snap = make_snapshot();
    beacon_battery_window_t window = make_batt_window();
    log_record_idle out;

    build_beacon_record(&out, &snap, &window);

    TEST_ASSERT(out.css_px0 == snap.css_px0 && out.css_px1 == snap.css_px1);
    TEST_ASSERT(out.css_nx0 == snap.css_nx0 && out.css_nx1 == snap.css_nx1);
    TEST_ASSERT(out.css_py0 == snap.css_py0 && out.css_py1 == snap.css_py1);
    TEST_ASSERT(out.css_ny0 == snap.css_ny0 && out.css_ny1 == snap.css_ny1);
    TEST_ASSERT(out.css_pz0 == snap.css_pz0 && out.css_pz1 == snap.css_pz1);
    TEST_ASSERT(out.css_nz0 == snap.css_nz0 && out.css_nz1 == snap.css_nz1);

    TEST_ASSERT(out.temp_px == snap.temp_px);
    TEST_ASSERT(out.temp_nx == snap.temp_nx);
    TEST_ASSERT(out.temp_py == snap.temp_py);
    TEST_ASSERT(out.temp_ny == snap.temp_ny);
}

static void test_attitude_only_set_on_success(void) {
    beacon_battery_window_t window = make_batt_window();

    beacon_sensor_snapshot_t ok_snap = make_snapshot();
    log_record_idle ok_out;
    build_beacon_record(&ok_out, &ok_snap, &window);
    TEST_ASSERT_DBL(ok_out.attitude.x1, 1, 1e-9);
    TEST_ASSERT_DBL(ok_out.attitude.z3, 9, 1e-9);

    beacon_sensor_snapshot_t fail_snap = make_snapshot();
    fail_snap.det_status = GET_ATTITUDE_MAG_FAILURE;
    log_record_idle fail_out;
    build_beacon_record(&fail_out, &fail_snap, &window);
    TEST_ASSERT_DBL(fail_out.attitude.x1, 0, 1e-9);
    TEST_ASSERT_DBL(fail_out.attitude.z3, 0, 1e-9);
}

static void test_eclipse_flag_normalized(void) {
    beacon_sensor_snapshot_t snap = make_snapshot();
    beacon_battery_window_t window = make_batt_window();
    log_record_idle out;

    snap.is_in_eclipse = 5; // any nonzero value should normalize to 1
    build_beacon_record(&out, &snap, &window);
    TEST_ASSERT(out.is_in_eclipse == 1);

    snap.is_in_eclipse = 0;
    build_beacon_record(&out, &snap, &window);
    TEST_ASSERT(out.is_in_eclipse == 0);
}

int main(void) {
    TEST_SUITE("beacon_record");
    RUN_TEST(test_passthrough_fields);
    RUN_TEST(test_unimplemented_fields_are_zero);
    RUN_TEST(test_error_msg_populated);
    RUN_TEST(test_css_temp_passthrough);
    RUN_TEST(test_attitude_only_set_on_success);
    RUN_TEST(test_eclipse_flag_normalized);
    TEST_RESULTS();
}
