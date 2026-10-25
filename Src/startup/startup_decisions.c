#include "startup/startup_decisions.h"

bool startup_is_first_boot(uint32_t sat_first_run_reg) {
  return sat_first_run_reg != FLAG_SAT_ALREADY_RAN;
}

bool startup_burn_wire_pending(uint32_t burn_wire_reg) {
  return burn_wire_reg != FLAG_BURN_WIRE_FIRED;
}

bool startup_wait_pending(uint32_t wait_30_reg) {
  return wait_30_reg != FLAG_WAIT_DONE;
}

uint16_t startup_remaining_wait_seconds(uint32_t now, uint32_t deadline) {
  if (deadline <= now) return 0;
  uint32_t remaining = deadline - now;
  return remaining > UINT16_MAX ? UINT16_MAX : (uint16_t)remaining;
}

MCUState startup_initial_next_state(bool burn_wire_pending) {
  return burn_wire_pending ? WAIT_POWER_ON : WAITING_FOR_UPLINK;
}

MCUState startup_decide_waiting_for_uplink(bool got_uplink, uint32_t now, uint32_t uplink_deadline) {
  if (got_uplink) return SAT_TO_GROUND_1;
  if (now >= uplink_deadline) return DIAGNOSTICS_MODE;
  return WAITING_FOR_UPLINK;
}

MCUState startup_decide_ground_to_sat_1(bool received_ground_signal) {
  return received_ground_signal ? SAT_TO_GROUND_2 : GROUND_TO_SAT_1;
}

MCUState startup_decide_diagnostics_mode(bool received_uplink) {
  return received_uplink ? SAT_TO_GROUND_1 : DIAGNOSTICS_MODE;
}
