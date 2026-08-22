#include "Beacon_tests.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "../Assertions/Ass.h"
#include "../Suites/suite.h"
#include "print_scan.h"

#include "PWRMON/INA226.h"
#include "IMU/ASM330LHH.h"
#include "MAG/QMC5883L.h"
#include "RTC/rtc.h"
#include "ADCS.h"
#include "FRAM/loggers_to_fram.h"

#include "scheduler/beacon/beacon_task.h"
#include "scheduler/beacon/beacon_stats.h"
#include "logging_records.h"

#define FRAM_SCRATCH_PAGE (FRAM_BEACON_STATS_PAGE + 10)

static void test_fram_roundtrip() {
    const uint8_t pattern[8] = {0xDE, 0xAD, 0xBE, 0xEF, 0xA5, 0x5A, 0x01, 0xFF};
    uint8_t readback[8] = {0};

    bool pushed = FRAM_push(FRAM_SCRATCH_PAGE, sizeof(pattern), (void *) pattern);
    assert_equal_int(pushed ? 1 : 0, 1);

    bool fetched = FRAM_fetch(FRAM_SCRATCH_PAGE, sizeof(readback), readback);
    assert_equal_int(fetched ? 1 : 0, 1);

    for (int i = 0; i < 8; i++) {
        assert_equal_int(readback[i], pattern[i]);
    }
}

static void test_battery_read_sane() {
    float v = pwrmon_getBusVoltage(BATMON_GPIO, BATMON_SCL, BATMON_SDA);
    printMsg("\tbatt_voltage = %f\r\n", v);

    // Generous sanity bounds, not a calibrated check - just rules out a
    // stuck-at-zero/garbage/disconnected sensor.
    assert_greater_than_float(v, 0.0f, 0.0f);
    assert_less_than_float(v, 20.0f, 0.0f);
}

static void test_imu_read_sane() {
    assert_equal_int(imu_isCommunicationWorking() ? 1 : 0, 1);

    float gx = imu_readGyro_X();
    float gy = imu_readGyro_Y();
    float gz = imu_readGyro_Z();
    printMsg("\tgyro = (%f, %f, %f)\r\n", gx, gy, gz);

    assert_greater_than_equal_float(gx, -2000.0f, 0.0f);
    assert_less_than_equal_float(gx, 2000.0f, 0.0f);
    assert_greater_than_equal_float(gy, -2000.0f, 0.0f);
    assert_less_than_equal_float(gy, 2000.0f, 0.0f);
    assert_greater_than_equal_float(gz, -2000.0f, 0.0f);
    assert_less_than_equal_float(gz, 2000.0f, 0.0f);
}

static void test_mag_read_sane() {
    mag_select(MAG0);
    int16_t mx = mag_read_X();
    int16_t my = mag_read_Y();
    int16_t mz = mag_read_Z();
    printMsg("\tmag = (%d, %d, %d)\r\n", mx, my, mz);

    // A stuck/disconnected sensor typically reads all-zero; Earth's field
    // should produce a nonzero reading on at least one axis.
    assert_equal_int((mx != 0 || my != 0 || mz != 0) ? 1 : 0, 1);
}

static void test_rtc_calendar_time_sane() {
    uint8_t hour, minute, second;
    uint8_t year, month, date, weekday;

    rtc_getTime(&hour, &minute, &second);
    rtc_getCalendar(&year, &month, &date, &weekday);
    printMsg("\t%02d/%02d/%02d %02d:%02d:%02d (weekday %d)\r\n",
              year, month, date, hour, minute, second, weekday);

    assert_less_than_int(hour, 24);
    assert_less_than_int(minute, 60);
    assert_less_than_int(second, 60);
    assert_greater_than_equal_int(month, 1);
    assert_less_than_equal_int(month, 12);
    assert_greater_than_equal_int(date, 1);
    assert_less_than_equal_int(date, 31);
    assert_greater_than_equal_int(weekday, 1);
    assert_less_than_equal_int(weekday, 7);
}

static void test_adcs_attitude_and_eclipse_no_crash() {
    mat3 attitude;
    adcs_get_attitude_status status = ADCS_get_attitude(&attitude);
    printMsg("\tADCS_get_attitude status = %d\r\n", (int) status);

    if (status == GET_ATTITUDE_SUCCESS) {
        assert_equal_int(isnan(attitude.x1) ? 1 : 0, 0);
        assert_equal_int(isnan(attitude.y2) ? 1 : 0, 0);
        assert_equal_int(isnan(attitude.z3) ? 1 : 0, 0);
    }
    // else: on a bench without a valid TLE/epoch, failure is expected -
    // logged above, not asserted as a hard failure.

    int eclipse = ADCS_is_in_eclipse();
    printMsg("\tADCS_is_in_eclipse = %d\r\n", eclipse);
    assert_greater_than_equal_int(eclipse, 0);
    assert_less_than_equal_int(eclipse, 1);
}

static void test_beacon_cycle_end_to_end() {
    // beacon_task_cycle()'s radio_push retry path calls vTaskDelay, which
    // requires the FreeRTOS scheduler to be running. TestDefinition.h now
    // dispatches every test through a trampoline task created before
    // vTaskStartScheduler() (see main.c's testTaskTrampoline), so this
    // runs in a real task context and vTaskDelay is safe here.
    log_record_idle record;
    bool sent = beacon_task_cycle(&record);
    printMsg("\tbeacon_task_cycle() radio_push result = %d\r\n", sent ? 1 : 0);
    printMsg("\tbuilt record: batt_level=%f batt_7_day_avg=%f time=%02d:%02d:%02d\r\n",
              record.batt_level, record.batt_7_day_avg,
              record.current_hour, record.current_minute, record.current_second);

    // radio_push's success is link-dependent (needs a paired receiver on
    // the bench), so it isn't asserted true here - only that the record
    // got built with today's actual time, as a sanity check that the
    // full pipeline ran end to end.
    assert_less_than_int(record.current_hour, 24);
}

void testFunction_Beacon() {
    UT_Suite suite;
    init_suite(&suite, 22, "Beacon hardware integration");

    add_test(&suite, test_fram_roundtrip, "FRAM page roundtrip");
    add_test(&suite, test_battery_read_sane, "Battery voltage read");
    add_test(&suite, test_imu_read_sane, "IMU gyro read");
    add_test(&suite, test_mag_read_sane, "Magnetometer read");
    add_test(&suite, test_rtc_calendar_time_sane, "RTC time/calendar read");
    add_test(&suite, test_adcs_attitude_and_eclipse_no_crash, "ADCS attitude/eclipse read");
    add_test(&suite, test_beacon_cycle_end_to_end, "Full beacon_task_cycle()");

    run_suite(&suite);
}
