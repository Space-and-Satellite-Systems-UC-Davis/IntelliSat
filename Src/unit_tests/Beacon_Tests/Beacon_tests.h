#pragma once

/**
 * On-target hardware integration checks for the beaconing pipeline -
 * exercises the real drivers beacon_task.c calls (battery, IMU, mag,
 * RTC, FRAM, ADCS attitude/eclipse) plus one full beacon_task_cycle().
 * Dispatched via Src/inc/TestDefinition.h (RUN_TEST=1, TEST_ID=22).
 *
 * This complements, not replaces, the host-side tests in
 * Src/scheduler/beacon/tests/ - those verify build_beacon_record()/
 * beacon_stats.c's logic in isolation with stubbed hardware; this
 * verifies the real drivers actually behave sanely on a physical board.
 */
void testFunction_Beacon();
