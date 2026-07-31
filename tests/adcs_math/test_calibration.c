#include "tests/framework.h"
#include "adcs_math/calibration.h"
#include <stdint.h>

#define TOL_F 1e-5f

// ── lowpass_filter ────────────────────────────────────────────────────────────

static void test_lowpass_nan_prev(void) {
    // NaN prevValue → return currValue unmodified
    float result = lowpass_filter(5.0f, (float)NAN, 0.5f);
    TEST_ASSERT_FLT(result, 5.0f, TOL_F);
}

static void test_lowpass_steady_state(void) {
    // curr == prev → result equals both regardless of filter constant
    float result = lowpass_filter(3.0f, 3.0f, 0.5f);
    TEST_ASSERT_FLT(result, 3.0f, TOL_F);
}

static void test_lowpass_fc_zero(void) {
    // fc = 0: (1-0)*curr + 0*prev = curr
    float result = lowpass_filter(4.0f, 2.0f, 0.0f);
    TEST_ASSERT_FLT(result, 4.0f, TOL_F);
}

static void test_lowpass_fc_one(void) {
    // fc = 1: 0*curr + 1*prev = prev
    float result = lowpass_filter(4.0f, 2.0f, 1.0f);
    TEST_ASSERT_FLT(result, 2.0f, TOL_F);
}

static void test_lowpass_fc_half(void) {
    // fc = 0.5: 0.5*curr + 0.5*prev = average
    float result = lowpass_filter(4.0f, 2.0f, 0.5f);
    TEST_ASSERT_FLT(result, 3.0f, TOL_F);
}

// ── get_sensor_calibration ───────────────────────────────────────────────────

static void test_sensor_cal_passthrough(void) {
    // offset=0, scalar=1, fc=0 → (curr+0)*1 = curr, filter passes through
    float result = get_sensor_calibration(10.0f, 5.0f, 0.0f, 1.0f, 0.0f);
    TEST_ASSERT_FLT(result, 10.0f, TOL_F);
}

static void test_sensor_cal_transform(void) {
    // curr=2, offset=1, scalar=2, fc=0 → (2+1)*2 = 6
    float result = get_sensor_calibration(2.0f, 1.0f, 1.0f, 2.0f, 0.0f);
    TEST_ASSERT_FLT(result, 6.0f, TOL_F);
}

static void test_sensor_cal_filter_applied(void) {
    // curr=2, offset=0, scalar=2 → transformed = 4
    // prev=4, fc=0.5 → 0.5*4 + 0.5*4 = 4
    float result = get_sensor_calibration(2.0f, 4.0f, 0.0f, 2.0f, 0.5f);
    TEST_ASSERT_FLT(result, 4.0f, TOL_F);
}

// ── get_delta_t ───────────────────────────────────────────────────────────────

static void test_delta_t_normal(void) {
    TEST_ASSERT(get_delta_t(100, 50) == 50);
    TEST_ASSERT(get_delta_t(50, 0)   == 50);
}

static void test_delta_t_same(void) {
    TEST_ASSERT(get_delta_t(50, 50) == 0);
}

static void test_delta_t_overflow(void) {
    // prev > curr indicates the millisecond counter wrapped around UINT64_MAX
    // expected: (UINT64_MAX - prev) + curr = (UINT64_MAX - (UINT64_MAX-10)) + 5 = 10+5 = 15
    uint64_t prev     = UINT64_MAX - 10;
    uint64_t curr     = 5;
    uint64_t expected = 15;
    TEST_ASSERT(get_delta_t(curr, prev) == expected);
}

int main(void) {
    TEST_SUITE("calibration");
    RUN_TEST(test_lowpass_nan_prev);
    RUN_TEST(test_lowpass_steady_state);
    RUN_TEST(test_lowpass_fc_zero);
    RUN_TEST(test_lowpass_fc_one);
    RUN_TEST(test_lowpass_fc_half);
    RUN_TEST(test_sensor_cal_passthrough);
    RUN_TEST(test_sensor_cal_transform);
    RUN_TEST(test_sensor_cal_filter_applied);
    RUN_TEST(test_delta_t_normal);
    RUN_TEST(test_delta_t_same);
    RUN_TEST(test_delta_t_overflow);
    TEST_RESULTS();
}
