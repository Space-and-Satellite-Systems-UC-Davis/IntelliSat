#ifndef STARTUP_H
#define STARTUP_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <globals.h>
#include <print_scan.h>
#include <PDB/power_distribution.h>
#include <Radio/Radio_intercom.h>

#include "startup/startup_reset.h"
#include "startup/startup_state.h"
#include "startup/startup_store.h"

/*
 * Electrical must supply the eligibility contract; no guessed voltage or flight
 * permission. The production default is false: nothing is deployed until that
 * gate is deliberately implemented.
 */
#ifndef STARTUP_BATTERY_ELIGIBLE
#define STARTUP_BATTERY_ELIGIBLE() false
#endif

#define STARTUP_BURN_MS 5000u        /* commanded ON time for one attempt */
#define STARTUP_RESPONSE_MS 60000u   /* wait for the ground ACK before re-HELLO */
#define STARTUP_HELLO_RETRY_MS 60000u
#define STARTUP_CHECKPOINT_MS 60000u /* at most this much counted time between saves */
#define STARTUP_POLL_MS 1000u        /* awake pacing while nothing else happens */

/* Local queue/transfer acceptance does not prove RF delivery or ground receipt. */
static uint8_t handshake_step1_msg[] = "HELLO";
static uint8_t handshake_step3_msg[] = "READY";
#define HANDSHAKE_TRANSMIT_STEP1() (radio_push(handshake_step1_msg, sizeof(handshake_step1_msg) - 1) && radio_downlink(NULL, 1))
#define HANDSHAKE_TRANSMIT_STEP3() (radio_push(handshake_step3_msg, sizeof(handshake_step3_msg) - 1) && radio_downlink(NULL, 1))

/* Exact role tokens only; these are not authentication or replay protection. */
static bool received_uplink(void) {
    uint8_t chunk[MAX_PAYLOAD_BYTES];
    RadioPacket packet = radio_force_pull(chunk);
    return packet.size > 0 && packet.size <= sizeof(chunk) &&
           packet.size == 5 && memcmp(chunk, "START", 5) == 0;
}

static bool received_ground_signal(void) {
    uint8_t chunk[MAX_PAYLOAD_BYTES];
    RadioPacket packet = radio_force_pull(chunk);
    return packet.size > 0 && packet.size <= sizeof(chunk) &&
           packet.size == 3 && memcmp(chunk, "ACK", 3) == 0;
}

/*
 * Fail closed: command OFF, then request a reset. There is no blocking log and
 * no delay before the request, so a storage fault cannot park the board with
 * deployment still armed. A persistent fault resets repeatedly by design.
 */
static void startup_fault(void) {
    pdb_pyro_off();
    startup_request_reset();
    /* Reached only if a reset is not taken: stay OFF, never enter the mission. */
    for (;;) delay_ms(STARTUP_POLL_MS);
}

/* Persist the handshake entry before transmitting, so a reset here can never
 * re-enable deployment or replay a burn that was already reserved. */
static void startup_enter_handshake(StartupStoreState *state) {
    state->resume_state = SAT_TO_GROUND_1;
    state->remaining_ms = 0;
    if (startup_store_save(state) != STARTUP_STORE_OK) startup_fault();
}

/*
 * Blocking wait for the time left in *state, measured with the boot-local
 * monotonic timer only. Every interval is counted exactly once: the reference
 * advances by the amount just subtracted from the RAM remaining, so work done
 * inside the loop (radio polling, FRAM access) can never be subtracted twice or
 * expire the wait early. Unknown power-off time is never counted and unsaved
 * progress is lost on reset by design.
 *
 * @param   state   stored record whose remaining_ms is counted down
 * @param   listen  poll for a START uplink, at least once a second
 * @returns true if a START uplink arrived, false when the wait is over
 */
static bool startup_wait(StartupStoreState *state, bool listen) {
    uint64_t reference = getSysTime();
    uint32_t remaining = state->remaining_ms;
    uint32_t unsaved = 0;

    for (;;) {
        if (listen && received_uplink()) return true;
        if (remaining == 0) return false;

        uint64_t elapsed = getSysTime() - reference;
        if (elapsed > 0) {
            uint32_t step = elapsed > (uint64_t)remaining ? remaining : (uint32_t)elapsed;
            reference += step;
            remaining -= step;
            unsaved += step;
            if (remaining == 0) {
                state->remaining_ms = 0;
                return false;
            }
        }
        if (unsaved >= STARTUP_CHECKPOINT_MS) {
            state->remaining_ms = remaining;
            if (startup_store_save(state) != STARTUP_STORE_OK) startup_fault();
            unsaved = 0;
        }

        /* Listening stays responsive; the save cadence above stays a minute. */
        uint32_t sleep = remaining;
        if (sleep > STARTUP_CHECKPOINT_MS - unsaved) sleep = STARTUP_CHECKPOINT_MS - unsaved;
        if (listen && sleep > STARTUP_POLL_MS) sleep = STARTUP_POLL_MS;
        delay_ms(sleep);
    }
}

/* A deliberately unarmed record stays OFF without countdown or Startup writes. */
static void startup_unarmed_park(void) {
    pdb_pyro_off();
    printMsg("Unarmed record: staying inhibited with pyro commanded OFF.\r\n");
    for (;;) delay_ms(STARTUP_POLL_MS);
}

/*
 * Blocking deployment sequence: inhibit, conservative attempt-before-ON,
 * handshake, then hand off to the scheduler.
 *
 * Startup stays awake throughout; the boot-local timer progresses without an
 * RTC wake path. All persistence goes through startup_store_load/save, and no
 * FRAM or radio operation happens while the pyro is commanded ON.
 */
void Startup(void) {
    StartupStoreState state = {0};
    bool hello_attempted = false;
    uint64_t hello_attempt_ms = 0;
    uint64_t response_start_ms = 0;

    printMsg("t = 0 minutes: deployment sequence started.\r\n");
    radio_init(); /* Existing UART API, not a direct RF-power GPIO. */
    if (startup_store_load(&state) != STARTUP_STORE_OK) startup_fault();

    if (!state.armed) startup_unarmed_park(); /* never returns */

    while (true) {
        MCUState resume = (MCUState)state.resume_state;
        switch (resume) {
        case INITIAL_CHECKS:
            printMsg("First boot. Arming the 30 minute inhibit.\r\n");
            (void)startup_wait(&state, false);
            state.resume_state = WAIT_POWER_ON;
            state.remaining_ms = 0;
            if (startup_store_save(&state) != STARTUP_STORE_OK) startup_fault();
            printMsg("t = 30 minutes: wait complete.\r\n");
            break;
        case WAIT_POWER_ON: {
            /* A queued START wins over a due retry. The first normal burn precedes
             * listening, so this only applies once an attempt was consumed. */
            bool retry = state.attempts_used > 0;
            if (retry && received_uplink()) {
                startup_enter_handshake(&state);
                break;
            }
            if (!STARTUP_BATTERY_ELIGIBLE()) break; /* stay OFF, no writes, keep polling */
            if (state.attempts_used < STARTUP_ATTEMPT_LIMIT) ++state.attempts_used;
            state.resume_state = WAITING_FOR_UPLINK;
            state.remaining_ms = startup_store_retry_ms(state.attempts_used);
            /* The consumed attempt and a full retry interval must be durable before ON. */
            if (startup_store_save(&state) != STARTUP_STORE_OK) startup_fault();
            /* A retry that receives START after the reservation commands OFF and enters
             * the saved handshake state without ON. A first attempt still burns. */
            if (retry && received_uplink()) {
                pdb_pyro_off();
                startup_enter_handshake(&state);
                break;
            }
            printMsg("burn wire on.\r\n");
            pdb_pyro_on();
            {
                /* Awake monotonic timing only: no FRAM and no radio while commanded ON.
                 * Timer/watchdog hardware is not proof of physical cutoff on a CPU hang. */
                uint64_t burn_start = getSysTime();
                while (getSysTime() - burn_start < STARTUP_BURN_MS) delay_ms(1);
            }
            pdb_pyro_off();
            /* The full retry interval is already saved; it starts counting from this OFF. */
            printMsg("Waiting handshake...\r\n");
            break;
        }
        case WAITING_FOR_UPLINK:
            if (startup_wait(&state, true)) {
                startup_enter_handshake(&state);
                printMsg("Uplink received. Perform handshake.\r\n");
                break;
            }
            /* Ineligible power keeps duty in the listening state, never in ON. */
            if (!STARTUP_BATTERY_ELIGIBLE()) break;
            state.resume_state = WAIT_POWER_ON;
            break;
        case SAT_TO_GROUND_1:
            if (hello_attempted && getSysTime() - hello_attempt_ms < STARTUP_HELLO_RETRY_MS) break;
            printMsg("Handshake step 1: satellite -> ground.\r\n");
            hello_attempted = true;
            hello_attempt_ms = getSysTime();
            if (HANDSHAKE_TRANSMIT_STEP1()) {
                printMsg("Handshake step 2: waiting for ground -> satellite confirmation.\r\n");
                response_start_ms = getSysTime();
                state.resume_state = GROUND_TO_SAT_1;
            } else {
                printMsg("HELLO local request failed; retry paced, no ground receipt claimed.\r\n");
            }
            break;
        case GROUND_TO_SAT_1:
            /* received_ground_signal() does the actual pull; never pull twice per iteration. */
            if (received_ground_signal()) {
                printMsg("Ground confirmation received.\r\n");
                state.resume_state = SAT_TO_GROUND_2;
            } else if (getSysTime() - response_start_ms >= STARTUP_RESPONSE_MS) {
                state.resume_state = SAT_TO_GROUND_1;
            }
            break;
        case SAT_TO_GROUND_2:
            printMsg("Handshake step 3: satellite confirms it can transmit.\r\n");
            /* The ground ACK establishes success; READY is a best-effort local request. */
            if (!HANDSHAKE_TRANSMIT_STEP3()) printMsg("READY local request failed; ground receipt unknown.\r\n");
            state.resume_state = MAIN_OPS;
            state.remaining_ms = 0;
            if (startup_store_save(&state) != STARTUP_STORE_OK) startup_fault();
            printMsg("Handshake complete. Entering main ops.\r\n");
            break;
        case DIAGNOSTICS_MODE:
            /* Never persisted; an in-RAM diagnostics detour continues as a retry wait. */
            printMsg("Diagnostics mode: continuing retry wait/listen.\r\n");
            state.resume_state = WAITING_FOR_UPLINK;
            break;
        case MAIN_OPS:
            /* TLE tracking and periodic downlink/uplink servicing are ADCS and comms
             * scheduler work: they run once the RTOS starts, not before it. */
            printMsg("Handshake complete. Handing off to scheduler.\r\n");
            return;
        default:
            /* Unreachable through validation; fail closed rather than guess. */
            startup_fault();
            return;
        }
        delay_ms(STARTUP_POLL_MS);
    }
}

#endif /* STARTUP_H */
