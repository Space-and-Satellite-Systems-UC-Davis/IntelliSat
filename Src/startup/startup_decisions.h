#ifndef STARTUP_DECISIONS_H
#define STARTUP_DECISIONS_H

#include <stdint.h>
#include <stdbool.h>

// Pure decision logic for the Startup() state machine (Src/startup/startup.h),
// deliberately kept free of hardware includes (RTC/PDB/PWR/Radio/STM32)
// so it can be compiled and unit tested on the host - see
// Src/startup/tests/. Startup() performs the actual hardware I/O and
// calls into these functions to decide what to do with the results.

#define FLAG_SAT_ALREADY_RAN 0x00000001
#define FLAG_WAIT_DONE 0x00000001
#define FLAG_BURN_WIRE_FIRED 0x00000001

typedef enum {
  INITIAL_CHECKS,     // initiate MCU
  WAIT_POWER_ON,      // wait until t=30 to power on
  WAITING_FOR_UPLINK, //(a) receive uplink
  SAT_TO_GROUND_1,    // perform handshake: sat to ground
  GROUND_TO_SAT_1,    // perform handshake: ground to sat
  SAT_TO_GROUND_2,    // perform handshake: sat to ground again
  MAIN_OPS,           // main ops after handshake
  DIAGNOSTICS_MODE    // receved nothing after 2 weeks
} MCUState;

bool startup_is_first_boot(uint32_t sat_first_run_reg);
bool startup_burn_wire_pending(uint32_t burn_wire_reg);
bool startup_wait_pending(uint32_t wait_30_reg);

/**
 * Seconds remaining until deadline, clamped to fit PWR_enterLPSleepMode's
 * uint16_t parameter. Returns 0 if deadline has already passed (deadline
 * <= now) - guarded before subtracting, so an elapsed deadline can't
 * underflow the uint32_t subtraction and get silently clamped to
 * UINT16_MAX (~18h) instead of correctly returning 0.
 */
uint16_t startup_remaining_wait_seconds(uint32_t now, uint32_t deadline);

MCUState startup_initial_next_state(bool burn_wire_pending);

MCUState startup_decide_waiting_for_uplink(bool got_uplink, uint32_t now, uint32_t uplink_deadline);
MCUState startup_decide_ground_to_sat_1(bool received_ground_signal);
MCUState startup_decide_diagnostics_mode(bool received_uplink);

#endif // STARTUP_DECISIONS_H
