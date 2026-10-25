#ifndef STARTUP_H
#define STARTUP_H

#include "PDB/power_distribution.h"
#include "Radio/Radio_intercom.h"
#include "startup/startup_clock.h"
#include "startup/startup_decisions.h"
#include "startup/startup_persistence.h"
#include <string.h>
#include <LED/led.h>
#include <globals.h>
#include <print_scan.h>

// Electrical must supply the eligibility contract; no guessed voltage or flight permission.
#ifndef STARTUP_BATTERY_ELIGIBLE
#define STARTUP_BATTERY_ELIGIBLE() false
#endif
#ifndef STARTUP_BURN_MS
#define STARTUP_BURN_MS 5000u
#endif
#ifndef STARTUP_RESPONSE_MS
#define STARTUP_RESPONSE_MS 60000u
#endif
#ifndef STARTUP_HELLO_RETRY_MS
#define STARTUP_HELLO_RETRY_MS 60000u
#endif
#define STARTUP_CHECKPOINT_MS 60000u
// Local queue/transfer acknowledgments do not prove RF delivery or ground receipt.
static uint8_t handshake_step1_msg[] = "HELLO";
static uint8_t handshake_step3_msg[] = "READY";
#define HANDSHAKE_TRANSMIT_STEP1() (radio_push(handshake_step1_msg, sizeof(handshake_step1_msg) - 1) && radio_downlink(NULL, 1))
#define HANDSHAKE_TRANSMIT_STEP3() (radio_push(handshake_step3_msg, sizeof(handshake_step3_msg) - 1) && radio_downlink(NULL, 1))
// Exact role tokens only; these are not authentication or replay protection.
bool received_uplink() {
    uint8_t chunk[MAX_PAYLOAD_BYTES];
    RadioPacket packet = radio_force_pull(chunk);
    return packet.size > 0 && packet.size <= sizeof(chunk) &&
           packet.size == 5 && memcmp(chunk, "START", 5) == 0;
}
bool received_ground_signal() {
    uint8_t chunk[MAX_PAYLOAD_BYTES];
    RadioPacket packet = radio_force_pull(chunk);
    return packet.size > 0 && packet.size <= sizeof(chunk) &&
           packet.size == 3 && memcmp(chunk, "ACK", 3) == 0;
}
static void startup_fatal(void) {
    pdb_pyro_off();
    printMsg("FATAL: startup inhibited; OFF commanded, no mission entry.\r\n");
    while (true);
}
// Fresh recovery only, never called on every wait measurement (st_load revokes cursors).
static void startup_load(StSnapshot *snapshot) {
    StResult result = st_load(snapshot);
    if (result == ST_RECOVERABLE) result = st_repair(snapshot, snapshot);
    if (result != ST_OK) startup_fatal();
}
void Startup() {
    //! At this point RTC init should be called
  MCUState state = INITIAL_CHECKS;
  StSnapshot snapshot;
  StWaitCursor cursor = {0};
  StClock clock;
  bool initialized = false, no_old_time = false;
  bool hello_attempted = false;
  uint64_t hello_attempt_ms = 0, response_start_ms = 0;

  printMsg("t = 0 minutes: deployment sequence started.\r\n");

  while (true) {
    switch (state) {
    case INITIAL_CHECKS: {
        if (!initialized) {
            pdb_init();
            pdb_pyro_off(); // Command only; reset polarity/electrical cutoff still require qualification.
            radio_init(); // Existing UART API, not a direct RF-power GPIO.
            startup_load(&snapshot);
            initialized = true;
        }
        clock = startup_clock_sample();
        if (!clock.valid) no_old_time = true;
        if (snapshot.control.phase == ST_PHASE_PREPARED) {
            if (snapshot.control.authorization != 1) startup_fatal();
            if (!clock.valid) break;
            printMsg("First boot. Arming 30 minute wait.\r\n");
            // Fresh interval entry: only an already witnessed later sample is a reference.
            clock = startup_clock_anchor();
            if (!clock.valid) break;
            if (st_start_inhibit(&snapshot, &clock, &snapshot) != ST_OK) startup_fatal();
        }
        if (snapshot.control.phase == ST_PHASE_RESERVED) {
            // Reset between reservation/ON/OFF consumes that reservation, never resumes a burn.
            pdb_pyro_off();
            // Fresh interval entry for the retry wait: a witnessed later sample, never
            // the pre-recovery sample.
            clock = startup_clock_anchor();
            if (!clock.valid) break;
            if (st_after_off(&snapshot, &clock, &snapshot) != ST_OK) startup_fatal();
        }
        if (snapshot.control.phase == ST_PHASE_INHIBIT) {
            while (snapshot.control.phase == ST_PHASE_INHIBIT) {
                clock = startup_clock_sample();
                if (!clock.valid) {
                    cursor.token = 0;
                    no_old_time = true;
                    delay_ms(1000);
                    continue;
                }
                if (!cursor.token) {
                    startup_load(&snapshot);
                    StEntry entry = !no_old_time && startup_clock_retained() ?
                        ST_BOOT_CONTINUITY_PROVEN : ST_NO_OLD_TIME_CREDIT;
                    // Entry reference must not predate the entry instant itself.
                    clock = startup_clock_anchor();
                    if (!clock.valid) {
                        no_old_time = true;
                        delay_ms(1000);
                        continue;
                    }
                    if (st_wait_open(&snapshot, &clock, entry, &cursor) != ST_OK) startup_fatal();
                    // Verify the no-old-credit entry barrier and then current progress.
                    if (st_checkpoint(&snapshot, &cursor, &clock, &snapshot) != ST_OK ||
                        st_checkpoint(&snapshot, &cursor, &clock, &snapshot) != ST_OK) startup_fatal();
                }
                uint64_t elapsed = 0;
                StResult result = st_wait_measure(&snapshot, &cursor, &clock, &elapsed);
                if (result == ST_CLOCK_INVALID || result == ST_REANCHOR_REQUIRED) {
                    cursor.token = 0;
                    no_old_time = true;
                    continue;
                }
                if (result != ST_OK) startup_fatal();
                if (st_checkpoint(&snapshot, &cursor, &clock, &snapshot) != ST_OK) startup_fatal();
                // Verify the durable clock domain before waiting or completing the inhibit.
                if (!startup_clock_confirm_wait(clock.source, clock.revision)) {
                    cursor.token = 0;
                    no_old_time = true;
                    delay_ms(1000);
                    continue;
                }
                if (elapsed >= snapshot.control.duration_ms) {
                    if (st_finish_inhibit(&snapshot, &cursor, &clock, &snapshot) != ST_OK) startup_fatal();
                    printMsg("t = 30 minutes: wait complete.\r\n");
                    break;
                }
                uint64_t remaining = snapshot.control.duration_ms - elapsed;
                if (remaining > STARTUP_CHECKPOINT_MS) remaining = STARTUP_CHECKPOINT_MS;
                // Remain awake: TIM7 keeps advancing and cannot strand the MCU in
                // an unqualified RTC wake path. Remeasure the calendar after each
                // at-most-60-second interval; only verified progress completes the wait.
                delay_ms(remaining);
            }
        }
        switch (snapshot.control.phase) {
        case ST_PHASE_ELIGIBLE: state = WAIT_POWER_ON; break;
        case ST_PHASE_RETRY_WAIT:
            printMsg("Waiting handshake...\r\n");
            state = WAITING_FOR_UPLINK; cursor.token = 0; break;
        case ST_PHASE_RETIRED: state = SAT_TO_GROUND_1; break;
        case ST_PHASE_COMPLETE: state = MAIN_OPS; break;
        default: startup_fatal();
        }
        break;
    }
    case WAIT_POWER_ON: {
        bool retry = snapshot.control.phase == ST_PHASE_RETRY_WAIT;
        // A queued START wins over a due retry, but the normal first burn precedes listening.
        if (retry && received_uplink()) {
            if (st_retire(&snapshot, &snapshot) != ST_OK) startup_fatal();
            state = SAT_TO_GROUND_1;
            break;
        }
        if (!STARTUP_BATTERY_ELIGIBLE()) {
            if (retry) state = WAITING_FOR_UPLINK;
            break; // No consumed reservation or reset of the wait lower bound.
        }
        clock = startup_clock_sample();
        if (!clock.valid) {
            if (retry) { cursor.token = 0; no_old_time = true; state = WAITING_FOR_UPLINK; }
            break;
        }
        printMsg("burn wire on.\r\n");
        StOnPermit permit = {0};
        StResult result = st_reserve(&snapshot, retry ? &cursor : NULL, &clock, &snapshot, &permit);
        if (result == ST_NOT_DUE) { state = WAITING_FOR_UPLINK; break; }
        if (result == ST_CLOCK_INVALID || result == ST_REANCHOR_REQUIRED) {
            cursor.token = 0; no_old_time = true; state = WAITING_FOR_UPLINK; break;
        }
        if (result != ST_OK) startup_fatal();
        // A late START can consume this reservation without ON. No storage call may precede take otherwise.
        if (retry && received_uplink()) {
            pdb_pyro_off();
            // Fresh interval entry: the reference must be witnessed after this OFF.
            clock = startup_clock_anchor();
            if (!clock.valid) { state = INITIAL_CHECKS; break; }
            if (st_after_off(&snapshot, &clock, &snapshot) != ST_OK ||
                st_retire(&snapshot, &snapshot) != ST_OK) startup_fatal();
            state = SAT_TO_GROUND_1;
            break;
        }
        if (st_take_on_permit(&snapshot, &permit) != ST_OK) startup_fatal();
        pdb_pyro_on();
        uint64_t burn_start = getSysTime();
        // Awake monotonic time only: no low-power sleep, FRAM or radio while commanded ON.
        // Timer/watchdog hardware is not proof of physical cutoff on a CPU hang.
        while (getSysTime() - burn_start < STARTUP_BURN_MS) delay_ms(1);
        pdb_pyro_off();
        // OFF-to-cooldown entry: the new wait reference is a witnessed later sample.
        clock = startup_clock_anchor();
        if (!clock.valid) { state = INITIAL_CHECKS; break; }
        if (st_after_off(&snapshot, &clock, &snapshot) != ST_OK) startup_fatal();
        cursor.token = 0;
        no_old_time = true;
        // OFF completion and the new two-day/weekly wait are one verified critical transition.
        printMsg("Waiting handshake...\r\n");
        state = WAITING_FOR_UPLINK;
        break;
    }
    case WAITING_FOR_UPLINK: {
        if (received_uplink()) {
            if (st_retire(&snapshot, &snapshot) != ST_OK) startup_fatal();
            printMsg("Uplink received. Peform handshake.\r\n");
            state = SAT_TO_GROUND_1;
            break;
        }
        clock = startup_clock_sample();
        if (!clock.valid) { cursor.token = 0; no_old_time = true; break; }
        if (!cursor.token) {
            startup_load(&snapshot);
            StEntry entry = !no_old_time && startup_clock_retained() ?
                ST_BOOT_CONTINUITY_PROVEN : ST_NO_OLD_TIME_CREDIT;
            // Retry-wait entry reference is anchored the same way as the inhibit entry.
            clock = startup_clock_anchor();
            if (!clock.valid) { no_old_time = true; break; }
            if (st_wait_open(&snapshot, &clock, entry, &cursor) != ST_OK) startup_fatal();
            if (st_checkpoint(&snapshot, &cursor, &clock, &snapshot) != ST_OK ||
                st_checkpoint(&snapshot, &cursor, &clock, &snapshot) != ST_OK) startup_fatal();
            // Both verified entry checkpoints are durable: acknowledge this boot's
            // reset evidence against the revalidated clock domain.
            if (!startup_clock_confirm_wait(clock.source, clock.revision)) {
                cursor.token = 0;
                no_old_time = true;
                break;
            }
        }
        uint64_t elapsed = 0;
        StResult result = st_wait_measure(&snapshot, &cursor, &clock, &elapsed);
        if (result == ST_CLOCK_INVALID || result == ST_REANCHOR_REQUIRED) {
            cursor.token = 0; no_old_time = true; break;
        }
        if (result != ST_OK) startup_fatal();
        uint64_t durable = snapshot.has_progress ? snapshot.progress.elapsed_ms : snapshot.control.elapsed_ms;
        if (elapsed - durable >= STARTUP_CHECKPOINT_MS ||
            (elapsed >= snapshot.control.duration_ms && durable < snapshot.control.duration_ms)) {
            if (st_checkpoint(&snapshot, &cursor, &clock, &snapshot) != ST_OK) startup_fatal();
        }
        if (elapsed >= snapshot.control.duration_ms) state = WAIT_POWER_ON;
        break;
    }
    case SAT_TO_GROUND_1:
        if (hello_attempted && getSysTime() - hello_attempt_ms < STARTUP_HELLO_RETRY_MS) break;
        printMsg("Handshake step 1: satellite -> ground.\r\n");
        hello_attempted = true;
        hello_attempt_ms = getSysTime();
        if (HANDSHAKE_TRANSMIT_STEP1()) {
            printMsg("Handshake step 2: waiting for ground -> satellite confirmation.\r\n");
            response_start_ms = getSysTime();
            state = GROUND_TO_SAT_1;
        } else {
            printMsg("HELLO local request failed; retry paced, no ground receipt claimed.\r\n");
        }
        break;
    case GROUND_TO_SAT_1:
        // received_ground_signal() does the actual pull; do not pull twice per iteration.
        if (received_ground_signal()) {
            printMsg("Ground confirmation received.\r\n");
            state = SAT_TO_GROUND_2;
        } else if (getSysTime() - response_start_ms >= STARTUP_RESPONSE_MS) {
            state = SAT_TO_GROUND_1;
        }
        break;
    case SAT_TO_GROUND_2:
        printMsg("Handshake step 3: satellite confirms knows can transmit.\r\n");
        // Expected ground ACK establishes success. READY is a best-effort local request.
        if (!HANDSHAKE_TRANSMIT_STEP3()) printMsg("READY local request failed; ground receipt unknown.\r\n");
        if (st_complete(&snapshot, &snapshot) != ST_OK) startup_fatal();
        printMsg("Handshake complete. Entering main ops.\r\n");
        state = MAIN_OPS;
        break;
    case DIAGNOSTICS_MODE:
        printMsg("Diagnostics mode: continuing retry wait/listen.\r\n");
        state = WAITING_FOR_UPLINK;
        break;
    case MAIN_OPS:
      // TLE tracking (good/bad TLE branching) is owned by ADCS, and
      // periodic downlink/uplink servicing is owned by the comms
      // scheduler task - both run once the RTOS scheduler starts, not as
      // part of this pre-scheduler startup sequence. Hand control back to
      // the caller (platform_init) so it can start the scheduler.
      // check TLE
      // Bad TLE / next upload + handshake (hs)
      // TLE not working accurately -> periodic downlinking (high bat / low bat)
      // listen for uplink -> then immediately downlink
      // Good TLE / next upload + hs works
      // periodic downlinking
      // listen for uplink -> immediate downlink [high bat / (low bat -> we
      // didn't know we are in comms window)] comms open -> immediate uplink
      // wait x min. for uplink
      // if nothing then downlink
      // Note: uplink and downlink can't be done within same period
      // (*) ground station should have ability to can command sat which window
      // to uplink or downlink (TBD)
      printMsg("Handshake complete. Handing off to scheduler.\r\n");
      return;
    }
    delay_ms(1000);
  }
}

#endif // STARTUP_H
