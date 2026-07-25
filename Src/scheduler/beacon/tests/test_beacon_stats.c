#include "framework.h"

#include "beacon_stats.h"
#include "loggers_to_fram.h" // for fram_stub_reset()

static void test_first_boot_zeroes_and_is_empty(void) {
    fram_stub_reset();
    beacon_stats_init();

    beacon_battery_window_t window;
    beacon_stats_get(&window);

    TEST_ASSERT_FLT(window.batt_low, 0.0f, 1e-6);
    TEST_ASSERT_FLT(window.batt_avg, 0.0f, 1e-6);
    TEST_ASSERT_FLT(window.batt_high, 0.0f, 1e-6);
}

static void test_single_reading(void) {
    fram_stub_reset();
    beacon_stats_init();

    beacon_stats_update(3.7f, /*day_number=*/100);

    beacon_battery_window_t window;
    beacon_stats_get(&window);

    TEST_ASSERT_FLT(window.batt_low, 3.7f, 1e-6);
    TEST_ASSERT_FLT(window.batt_avg, 3.7f, 1e-6);
    TEST_ASSERT_FLT(window.batt_high, 3.7f, 1e-6);
}

static void test_same_day_accumulates_low_high_avg(void) {
    fram_stub_reset();
    beacon_stats_init();

    beacon_stats_update(3.0f, 100);
    beacon_stats_update(5.0f, 100);
    beacon_stats_update(4.0f, 100);

    beacon_battery_window_t window;
    beacon_stats_get(&window);

    TEST_ASSERT_FLT(window.batt_low, 3.0f, 1e-6);
    TEST_ASSERT_FLT(window.batt_high, 5.0f, 1e-6);
    TEST_ASSERT_FLT(window.batt_avg, 4.0f, 1e-6);
}

static void test_bucket_rotation_resets_after_7_days(void) {
    fram_stub_reset();
    beacon_stats_init();

    // Day 100 lands in the same bucket (100 % 7 == 2) as day 107.
    beacon_stats_update(10.0f, 100);
    beacon_stats_update(20.0f, 100);

    // 7 days later: same bucket slot, but a different day_number -> stale, resets.
    beacon_stats_update(3.0f, 107);

    beacon_battery_window_t window;
    beacon_stats_get(&window);

    // Only the day-107 reading should remain for this bucket.
    TEST_ASSERT_FLT(window.batt_low, 3.0f, 1e-6);
    TEST_ASSERT_FLT(window.batt_high, 3.0f, 1e-6);
    TEST_ASSERT_FLT(window.batt_avg, 3.0f, 1e-6);
}

static void test_multiple_days_aggregate(void) {
    fram_stub_reset();
    beacon_stats_init();

    beacon_stats_update(3.0f, 100);
    beacon_stats_update(5.0f, 101);

    beacon_battery_window_t window;
    beacon_stats_get(&window);

    TEST_ASSERT_FLT(window.batt_low, 3.0f, 1e-6);
    TEST_ASSERT_FLT(window.batt_high, 5.0f, 1e-6);
    TEST_ASSERT_FLT(window.batt_avg, 4.0f, 1e-6); // (3+5)/2, one sample each
}

/*
 * Regression test for the bug in the previous (weekday + day-of-month)
 * design: two days that are a multiple of 7 *weeks* apart share both the
 * same weekday and the same day-of-month, and would have been wrongly
 * treated as "still today" (accumulating instead of resetting). Absolute
 * day_number can never repeat, so this must always reset correctly no
 * matter how far apart the two updates are, as long as they land in the
 * same bucket slot (day_number % 7 equal).
 */
static void test_far_apart_days_in_same_bucket_still_reset(void) {
    fram_stub_reset();
    beacon_stats_init();

    uint32_t day_a = 100;
    uint32_t day_b = day_a + (7 * 49); // 49 weeks later, same bucket slot (mod 7)

    beacon_stats_update(10.0f, day_a);
    beacon_stats_update(20.0f, day_a);

    beacon_stats_update(3.0f, day_b);

    beacon_battery_window_t window;
    beacon_stats_get(&window);

    TEST_ASSERT_FLT(window.batt_low, 3.0f, 1e-6);
    TEST_ASSERT_FLT(window.batt_high, 3.0f, 1e-6);
    TEST_ASSERT_FLT(window.batt_avg, 3.0f, 1e-6);
}

static void test_reinit_after_data_written_does_not_reset(void) {
    fram_stub_reset();
    beacon_stats_init();
    beacon_stats_update(3.7f, 100);

    // Simulate a warm reboot: init() runs again, magic value already set,
    // so persisted data must survive.
    beacon_stats_init();

    beacon_battery_window_t window;
    beacon_stats_get(&window);

    TEST_ASSERT_FLT(window.batt_low, 3.7f, 1e-6);
    TEST_ASSERT_FLT(window.batt_avg, 3.7f, 1e-6);
    TEST_ASSERT_FLT(window.batt_high, 3.7f, 1e-6);
}

int main(void) {
    TEST_SUITE("beacon_stats");
    RUN_TEST(test_first_boot_zeroes_and_is_empty);
    RUN_TEST(test_single_reading);
    RUN_TEST(test_same_day_accumulates_low_high_avg);
    RUN_TEST(test_bucket_rotation_resets_after_7_days);
    RUN_TEST(test_multiple_days_aggregate);
    RUN_TEST(test_far_apart_days_in_same_bucket_still_reset);
    RUN_TEST(test_reinit_after_data_written_does_not_reset);
    TEST_RESULTS();
}
