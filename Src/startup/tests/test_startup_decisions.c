#include "framework.h"

#include "../startup_decisions.h"

static void test_is_first_boot_true(void) {
  TEST_ASSERT(startup_is_first_boot(0) == true);
}

static void test_is_first_boot_false(void) {
  TEST_ASSERT(startup_is_first_boot(FLAG_SAT_ALREADY_RAN) == false);
}

static void test_burn_wire_pending_true(void) {
  TEST_ASSERT(startup_burn_wire_pending(0) == true);
}

static void test_burn_wire_pending_false(void) {
  TEST_ASSERT(startup_burn_wire_pending(FLAG_BURN_WIRE_FIRED) == false);
}

static void test_wait_pending_true(void) {
  TEST_ASSERT(startup_wait_pending(0) == true);
}

static void test_wait_pending_false(void) {
  TEST_ASSERT(startup_wait_pending(FLAG_WAIT_DONE) == false);
}

static void test_remaining_seconds_normal(void) {
  TEST_ASSERT(startup_remaining_wait_seconds(1000, 1500) == 500);
}

static void test_remaining_seconds_already_elapsed(void) {
  TEST_ASSERT(startup_remaining_wait_seconds(2000, 1500) == 0);
}

static void test_remaining_seconds_exactly_zero(void) {
  TEST_ASSERT(startup_remaining_wait_seconds(1500, 1500) == 0);
}

static void test_remaining_seconds_clamp(void) {
  TEST_ASSERT(startup_remaining_wait_seconds(0, 100000) == UINT16_MAX);
}

static void test_remaining_seconds_clamp_boundary(void) {
  TEST_ASSERT(startup_remaining_wait_seconds(0, UINT16_MAX) == UINT16_MAX);
  TEST_ASSERT(startup_remaining_wait_seconds(0, (uint32_t)UINT16_MAX + 1) == UINT16_MAX);
}

static void test_initial_next_state_burn_pending(void) {
  TEST_ASSERT(startup_initial_next_state(true) == WAIT_POWER_ON);
}

static void test_initial_next_state_burn_done(void) {
  TEST_ASSERT(startup_initial_next_state(false) == WAITING_FOR_UPLINK);
}

static void test_waiting_for_uplink_got_uplink(void) {
  TEST_ASSERT(startup_decide_waiting_for_uplink(true, 1000, 2000) == SAT_TO_GROUND_1);
}

static void test_waiting_for_uplink_deadline_passed(void) {
  TEST_ASSERT(startup_decide_waiting_for_uplink(false, 2000, 1000) == DIAGNOSTICS_MODE);
}

static void test_waiting_for_uplink_neither(void) {
  TEST_ASSERT(startup_decide_waiting_for_uplink(false, 1000, 2000) == WAITING_FOR_UPLINK);
}

static void test_waiting_for_uplink_both_true(void) {
  // Uplink received takes precedence, matching the original if/else-if
  // ordering (the deadline is never re-checked once uplink is true).
  TEST_ASSERT(startup_decide_waiting_for_uplink(true, 2000, 1000) == SAT_TO_GROUND_1);
}

static void test_waiting_for_uplink_deadline_exactly_now(void) {
  TEST_ASSERT(startup_decide_waiting_for_uplink(false, 1000, 1000) == DIAGNOSTICS_MODE);
}

static void test_ground_to_sat_1_signal_received(void) {
  TEST_ASSERT(startup_decide_ground_to_sat_1(true) == SAT_TO_GROUND_2);
}

static void test_ground_to_sat_1_no_signal(void) {
  TEST_ASSERT(startup_decide_ground_to_sat_1(false) == GROUND_TO_SAT_1);
}

static void test_diagnostics_mode_uplink_received(void) {
  TEST_ASSERT(startup_decide_diagnostics_mode(true) == SAT_TO_GROUND_1);
}

static void test_diagnostics_mode_no_uplink(void) {
  TEST_ASSERT(startup_decide_diagnostics_mode(false) == DIAGNOSTICS_MODE);
}

int main(void) {
  TEST_SUITE("startup_decisions");

  RUN_TEST(test_is_first_boot_true);
  RUN_TEST(test_is_first_boot_false);

  RUN_TEST(test_burn_wire_pending_true);
  RUN_TEST(test_burn_wire_pending_false);

  RUN_TEST(test_wait_pending_true);
  RUN_TEST(test_wait_pending_false);

  RUN_TEST(test_remaining_seconds_normal);
  RUN_TEST(test_remaining_seconds_already_elapsed);
  RUN_TEST(test_remaining_seconds_exactly_zero);
  RUN_TEST(test_remaining_seconds_clamp);
  RUN_TEST(test_remaining_seconds_clamp_boundary);

  RUN_TEST(test_initial_next_state_burn_pending);
  RUN_TEST(test_initial_next_state_burn_done);

  RUN_TEST(test_waiting_for_uplink_got_uplink);
  RUN_TEST(test_waiting_for_uplink_deadline_passed);
  RUN_TEST(test_waiting_for_uplink_neither);
  RUN_TEST(test_waiting_for_uplink_both_true);
  RUN_TEST(test_waiting_for_uplink_deadline_exactly_now);

  RUN_TEST(test_ground_to_sat_1_signal_received);
  RUN_TEST(test_ground_to_sat_1_no_signal);

  RUN_TEST(test_diagnostics_mode_uplink_received);
  RUN_TEST(test_diagnostics_mode_no_uplink);

  TEST_RESULTS();
}
