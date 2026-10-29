/*
 * Actual Startup() against peripheral boundaries: memory FRAM, modeled PDB and
 * radio, boot-local monotonic time, and a reset boundary the harness observes.
 * Startup() is included exactly once and unmodified; startup_store.c is linked
 * as a translation unit, so the byte codec under test is the real one.
 *
 * The modeled clock starts at zero on every modeled boot. Nothing here proves
 * RTC behaviour, RF delivery, electrical cutoff or a real board. Expected
 * persistent records are stated with the independent SSP1 fixture encoder,
 * never with the production encoder.
 */
#include <inttypes.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "framework.h"
#include "FRAM/MB85RS256B.h"
#include "startup/startup.h"
#include "startup_fixtures.h"

#define FRAM_BYTES 32768u
#define SENTINEL 0x5Au
#define BOOT_COUNTER_VALUE 0x11223344u
#define ADCS_MASK_VALUE 0x00000005u
/* Awake polling adds at most a few STARTUP_POLL_MS before an action happens. */
#define SLOP_MS (3u * STARTUP_POLL_MS)

static uint8_t fram[FRAM_BYTES];
static uint64_t now_ms;
static uint64_t largest_delay_ms;
static bool pyro_on;
static unsigned on_count, off_count, reset_count, write_count;
static bool fail_read, fail_write, fail_wren, fail_wrdi, ignore_writes;
static long cut_budget = -1;
static bool power_lost;
static uint64_t stop_at;
static bool battery_eligible;
static jmp_buf run_env;

typedef enum { INIT, OFF, ON, RX, HELLO, READY, DOWNLINK, PRINT, RESET, RETURNED } Action;
typedef struct { Action action; uint64_t at_ms; } Event;
static Event events[8192];
static size_t event_count;

typedef struct {
    uint32_t sequence;
    uint8_t state;
    uint8_t attempts;
    bool armed;
    uint32_t remaining;
    uint64_t at_ms;
} Save;
static Save saves[32768];
static size_t save_count;

typedef struct { const char *bytes; size_t size; uint64_t not_before; } Input;
static Input inputs[64];
static size_t input_count, input_pos;
static unsigned pulls, start_available_at_pull;
/* Modeled radio work time: real I/O advances the boot-local timer too. */
static uint64_t poll_cost_ms;
static unsigned delay_calls;

static void require(bool condition, const char *why) {
    if (!condition) {
        fprintf(stderr, "startup harness: %s\n", why);
        exit(2);
    }
}

static void event(Action action) {
    require(event_count < sizeof(events) / sizeof(events[0]), "event capacity");
    events[event_count++] = (Event){action, now_ms};
}

static size_t find_event(Action action, size_t from) {
    for (size_t i = from; i < event_count; ++i) if (events[i].action == action) return i;
    return event_count;
}

static unsigned count_events(Action action) {
    unsigned count = 0;
    for (size_t i = 0; i < event_count; ++i) count += events[i].action == action;
    return count;
}

static void require_order(const Action *actions, size_t size) {
    size_t position = 0;
    for (size_t i = 0; i < size; ++i) {
        position = find_event(actions[i], position);
        if (position >= event_count) {
            printf("  missing event %d in order\n", (int)actions[i]);
            TEST_ASSERT(false);
            return;
        }
        ++position;
    }
    TEST_ASSERT(true);
}

/***************************** Persistent media ******************************/

static bool media_slot_valid(uint8_t slot, Ssp1Record *record) {
    return ssp1_read(fram + SSP1_ADDRESS + slot * SSP1_SLOT_SIZE, record);
}

/* The newer committed record, as a fresh boot would select it. */
static bool media_current(Ssp1Record *record) {
    Ssp1Record a, b;
    bool valid_a = media_slot_valid(0, &a);
    bool valid_b = media_slot_valid(1, &b);
    if (valid_a && valid_b) {
        *record = a.sequence >= b.sequence ? a : b;
        return true;
    }
    if (valid_a) { *record = a; return true; }
    if (valid_b) { *record = b; return true; }
    return false;
}

static void media_reset(void) {
    memset(fram, SENTINEL, sizeof(fram));
    memcpy(fram, &(uint32_t){BOOT_COUNTER_VALUE}, sizeof(uint32_t));
    memcpy(fram + 4, &(uint32_t){ADCS_MASK_VALUE}, sizeof(uint32_t));
}

static void media_store(uint8_t state, uint8_t attempts, bool armed, uint32_t remaining) {
    ssp1_build(fram + SSP1_ADDRESS, 1, state, attempts, armed, remaining);
    ssp1_build(fram + SSP1_ADDRESS + SSP1_SLOT_SIZE, 1, state, attempts, armed, remaining);
}

static void require_media(uint8_t state, uint8_t attempts, uint32_t remaining) {
    Ssp1Record record;
    if (!media_current(&record)) {
        require(false, "media holds no committed record");
    }
    if (record.state != state || record.attempts != attempts || record.remaining != remaining) {
        printf("  media is state=%u attempts=%u remaining=%" PRIu32
               ", expected state=%u attempts=%u remaining=%" PRIu32 "\n",
               record.state, record.attempts, record.remaining, state, attempts, remaining);
        TEST_ASSERT(false);
    } else {
        TEST_ASSERT(true);
    }
}

static void require_boot_and_adcs_preserved(void) {
    uint32_t boot = 0, adcs = 0;
    memcpy(&boot, fram, sizeof(boot));
    memcpy(&adcs, fram + 4, sizeof(adcs));
    TEST_ASSERT(boot == BOOT_COUNTER_VALUE);
    TEST_ASSERT(adcs == ADCS_MASK_VALUE);
    for (size_t address = SSP1_ADDRESS + SSP1_IMAGE_SIZE; address < FRAM_BYTES; ++address) {
        if (fram[address] != SENTINEL) {
            TEST_ASSERT(false);
            return;
        }
    }
    TEST_ASSERT(true);
}

/***************************** Hardware boundaries ***************************/

/* Electrical eligibility boundary: modeled, never a voltage or flight claim. */
bool startup_test_battery_eligible(void) {
    return battery_eligible;
}

void pdb_init(void) {
    pyro_on = false;
    event(INIT);
}

void pdb_pyro_on(void) {
    require(!pyro_on, "pyro commanded ON while already ON");
    /* The consumed attempt and a complete retry interval must already be durable. */
    Ssp1Record record;
    require(media_current(&record), "ON before any committed record");
    require(record.state == WAITING_FOR_UPLINK && record.attempts >= 1,
            "ON before the attempt was saved as a retry wait");
    require(record.remaining == startup_store_retry_ms(record.attempts),
            "ON before a full retry interval was saved");
    pyro_on = true;
    ++on_count;
    event(ON);
}

void pdb_pyro_off(void) {
    pyro_on = false;
    ++off_count;
    event(OFF);
}

uint64_t getSysTime(void) {
    return now_ms;
}

void delay_ms(uint64_t ms) {
    if (ms > largest_delay_ms) largest_delay_ms = ms;
    require(ms > 0 && ms <= STARTUP_CHECKPOINT_MS, "awake pacing stays bounded");
    ++delay_calls;
    now_ms += ms;
    if (stop_at != 0 && now_ms >= stop_at) longjmp(run_env, 2);
}

int printMsg(const char *message, ...) {
    (void)message;
    event(PRINT);
    return 0;
}

void startup_request_reset(void) {
    ++reset_count;
    event(RESET);
    longjmp(run_env, 1);
}

RadioPacket radio_force_pull(uint8_t *chunk, size_t capacity) {
    require(capacity == MAX_PAYLOAD_BYTES, "diagnostic receive must pass its real buffer size");
    require(!pyro_on, "radio activity while the pyro is commanded ON");
    ++pulls;
    now_ms += poll_cost_ms;
    if (pulls < start_available_at_pull) return (RadioPacket){0, 0};
    if (input_pos >= input_count || now_ms < inputs[input_pos].not_before) return (RadioPacket){0, 0};
    Input input = inputs[input_pos++];
    if (input.size > 0 && input.size <= capacity && input.bytes != NULL) {
        memcpy(chunk, input.bytes, input.size);
    }
    event(RX);
    return (RadioPacket){input.size, 'u'};
}

bool radio_push(uint8_t *chunk, size_t size) {
    require(!pyro_on, "radio activity while the pyro is commanded ON");
    bool hello = size == 5 && memcmp(chunk, "HELLO", 5) == 0;
    bool ready = size == 5 && memcmp(chunk, "READY", 5) == 0;
    require(hello || ready, "handshake sends only the exact HELLO/READY tokens");
    if (hello) {
        Ssp1Record record;
        require(media_current(&record), "HELLO without a committed record");
        require(record.state == SAT_TO_GROUND_1, "HELLO before the handshake entry was saved");
    }
    event(hello ? HELLO : READY);
    return true;
}

bool radio_downlink(uint8_t *chunk, size_t count) {
    (void)chunk;
    require(count == 1, "a short message is one chunk");
    event(DOWNLINK);
    return true;
}

void radio_init(void) {
    require(!pyro_on, "radio init while the pyro is commanded ON");
}

bool FRAM_readData(uint16_t address, uint8_t *buffer, uint16_t size) {
    if (fail_read) return false;
    require(address + size <= FRAM_BYTES, "FRAM read bounds");
    memcpy(buffer, fram + address, size);
    return true;
}

bool FRAM_writeEnable(void) {
    ++write_count;
    return !fail_wren;
}

bool FRAM_writeDisable(void) {
    return !fail_wrdi;
}

bool FRAM_writeData(uint16_t address, uint8_t *data, uint16_t size) {
    require(!pyro_on, "storage write while the pyro is commanded ON");
    require(address >= SSP1_ADDRESS && address + size <= SSP1_ADDRESS + SSP1_IMAGE_SIZE,
            "storage writes stay inside bytes 8..47");
    require(size > 0, "non-empty write");
    if (fail_write) return false;
    if (cut_budget >= 0) {
        if (power_lost) return false;
        size_t applied = (size_t)cut_budget < size ? (size_t)cut_budget : size;
        memcpy(fram + address, data, applied);
        cut_budget -= (long)applied;
        if (applied < size) { power_lost = true; return false; }
        if (cut_budget == 0) power_lost = true;
        return true;
    }
    if (ignore_writes) return true;
    memcpy(fram + address, data, size);
    /* A publication always commits with a final single-byte 0xA5 write. */
    if (size == 1 && data[0] == 0xA5u && (address == SSP1_ADDRESS + SSP1_COMMIT ||
                                          address == SSP1_ADDRESS + SSP1_SLOT_SIZE + SSP1_COMMIT)) {
        uint8_t slot = address == SSP1_ADDRESS + SSP1_COMMIT ? 0 : 1;
        Ssp1Record record;
        require(media_slot_valid(slot, &record), "a committed record must decode");
        require(record.state != GROUND_TO_SAT_1 && record.state != SAT_TO_GROUND_2 &&
                record.state != DIAGNOSTICS_MODE, "transient handshake state was persisted");
        require(save_count < sizeof(saves) / sizeof(saves[0]), "save capacity");
        saves[save_count++] = (Save){record.sequence, record.state, record.attempts,
                                     record.flags == 0x01u, record.remaining, now_ms};
    }
    return true;
}

/***************************** Scenarios *************************************/

static void scenario(uint8_t state, uint8_t attempts, bool armed, uint32_t remaining) {
    media_reset();
    media_store(state, attempts, armed, remaining);
    now_ms = 0;
    pyro_on = false;
    on_count = off_count = reset_count = write_count = 0;
    fail_read = fail_write = fail_wren = fail_wrdi = ignore_writes = false;
    cut_budget = -1;
    power_lost = false;
    stop_at = 0;
    battery_eligible = true;
    poll_cost_ms = 0;
    delay_calls = 0;
    event_count = 0;
    save_count = 0;
    input_count = input_pos = pulls = start_available_at_pull = 0;
}

static void input_at(const char *bytes, size_t size, uint64_t not_before) {
    require(input_count < sizeof(inputs) / sizeof(inputs[0]), "input capacity");
    inputs[input_count++] = (Input){bytes, size, not_before};
}

static void input(const char *bytes) {
    input_at(bytes, bytes != NULL ? strlen(bytes) : 0, 0);
}

static void model_reboot(void) {
    now_ms = 0;
    pyro_on = false;
    stop_at = 0;
    event_count = 0;
    input_count = input_pos = pulls = start_available_at_pull = 0;
}

/* init_init() owns the boot-time safe output state, so every modeled boot
 * replays that prefix before Startup() runs. */
static int model_run(void) {
    pdb_init();
    pdb_pyro_off();
    int jumped = setjmp(run_env);
    if (jumped == 0) {
        Startup();
        event(RETURNED);
        return 0;
    }
    return jumped;
}

void test_startup_inhibit_saves_attempt_before_on_and_completes(void) {
    scenario(INITIAL_CHECKS, 0, true, STARTUP_INHIBIT_MS);
    /* The uplink arrives only after the first burn: the normal attempt precedes listening. */
    input_at("START", 5, STARTUP_INHIBIT_MS + STARTUP_BURN_MS);
    input("ACK");
    TEST_ASSERT(model_run() == 0);

    const Action order[] = {INIT, OFF, ON, OFF, RX, HELLO, DOWNLINK, RX, READY, DOWNLINK, RETURNED};
    require_order(order, sizeof(order) / sizeof(order[0]));
    TEST_ASSERT(count_events(ON) == 1);
    size_t on = find_event(ON, 0), off = find_event(OFF, on + 1);
    TEST_ASSERT(on < event_count && off < event_count);
    TEST_ASSERT(events[on].at_ms >= STARTUP_INHIBIT_MS);
    TEST_ASSERT(events[on].at_ms <= STARTUP_INHIBIT_MS + SLOP_MS);
    TEST_ASSERT(events[off].at_ms - events[on].at_ms == STARTUP_BURN_MS);

    /* One checkpoint per counted minute of the inhibit, then the transition. */
    unsigned inhibit_saves = 0;
    uint64_t previous_at = 0;
    for (size_t i = 0; i < save_count; ++i) {
        if (saves[i].state != INITIAL_CHECKS) continue;
        ++inhibit_saves;
        TEST_ASSERT(saves[i].remaining == STARTUP_INHIBIT_MS - inhibit_saves * STARTUP_CHECKPOINT_MS);
        TEST_ASSERT(saves[i].attempts == 0);
        TEST_ASSERT(saves[i].armed);
        if (previous_at != 0) TEST_ASSERT(saves[i].at_ms - previous_at <= STARTUP_CHECKPOINT_MS);
        previous_at = saves[i].at_ms;
    }
    TEST_ASSERT(inhibit_saves == STARTUP_INHIBIT_MS / STARTUP_CHECKPOINT_MS - 1u);

    /* The consumed attempt and the full retry interval are published before ON. */
    bool seen_attempt = false;
    for (size_t i = 0; i < save_count; ++i) {
        if (saves[i].state == WAITING_FOR_UPLINK) {
            TEST_ASSERT(saves[i].attempts == 1);
            TEST_ASSERT(saves[i].remaining == STARTUP_RETRY_MS);
            seen_attempt = true;
            break;
        }
    }
    TEST_ASSERT(seen_attempt);

    /* Completion is durable before the handoff and carries no transient state. */
    Ssp1Record final_record;
    TEST_ASSERT(media_current(&final_record));
    TEST_ASSERT(final_record.state == MAIN_OPS);
    TEST_ASSERT(final_record.attempts == 1);
    TEST_ASSERT(final_record.remaining == 0);
    TEST_ASSERT(final_record.flags == 0x01u);
    TEST_ASSERT(saves[save_count - 1].state == MAIN_OPS);
    require_boot_and_adcs_preserved();
}

void test_startup_resume_never_credits_unknown_time(void) {
    /* One millisecond of saved inhibit is still waited in full. */
    scenario(INITIAL_CHECKS, 0, true, 1);
    input_at("START", 5, 5000);
    input("ACK");
    TEST_ASSERT(model_run() == 0);
    size_t on = find_event(ON, 0);
    TEST_ASSERT(on < event_count);
    TEST_ASSERT(events[on].at_ms >= 1 && events[on].at_ms <= 1 + SLOP_MS);

    /* Saved remaining time is waited in full: time spent off is never credited. */
    scenario(INITIAL_CHECKS, 0, true, 1500000);
    now_ms = 900000000;
    input_at("START", 5, 900000000 + 1500000 + STARTUP_BURN_MS);
    input("ACK");
    TEST_ASSERT(model_run() == 0);
    on = find_event(ON, 0);
    TEST_ASSERT(on < event_count);
    TEST_ASSERT(events[on].at_ms >= 900000000 + 1500000);
    TEST_ASSERT(events[on].at_ms <= 900000000 + 1500000 + SLOP_MS);

    /* A finished inhibit proceeds straight to the attempt. */
    scenario(WAIT_POWER_ON, 0, true, 0);
    input_at("START", 5, 5000);
    input("ACK");
    TEST_ASSERT(model_run() == 0);
    on = find_event(ON, 0);
    TEST_ASSERT(on < event_count && events[on].at_ms <= SLOP_MS);
    TEST_ASSERT(count_events(ON) == 1);

    /* Saved cooldown progress is counted from the checkpoint, not from zero. */
    scenario(WAITING_FOR_UPLINK, 1, true, 300000);
    input_at("START", 5, 300000 + STARTUP_BURN_MS);
    input("ACK");
    TEST_ASSERT(model_run() == 0);
    on = find_event(ON, 0);
    TEST_ASSERT(on < event_count);
    TEST_ASSERT(events[on].at_ms >= 300000 && events[on].at_ms <= 300000 + SLOP_MS);
    require_media(MAIN_OPS, 2, 0);
}

void test_startup_interval_boundaries_and_attempt_saturation(void) {
    /* Fifth attempt used: the next interval is two days and the count advances. */
    scenario(WAIT_POWER_ON, 5, true, 0);
    stop_at = STARTUP_RETRY_MS + 2u * STARTUP_BURN_MS + SLOP_MS;
    TEST_ASSERT(model_run() == 2);
    TEST_ASSERT(count_events(ON) == 2);
    size_t first = find_event(ON, 0), second = find_event(ON, first + 1);
    TEST_ASSERT(second < event_count);
    TEST_ASSERT(events[second].at_ms - events[first].at_ms >= STARTUP_BURN_MS + STARTUP_RETRY_MS);
    TEST_ASSERT(events[second].at_ms - events[first].at_ms <= STARTUP_BURN_MS + STARTUP_RETRY_MS + SLOP_MS);
    require_media(WAITING_FOR_UPLINK, 7, STARTUP_WEEKLY_MS);

    /* Seventh attempt used: the interval is one week from then on. */
    scenario(WAIT_POWER_ON, 6, true, 0);
    stop_at = STARTUP_WEEKLY_MS + 2u * STARTUP_BURN_MS + SLOP_MS;
    TEST_ASSERT(model_run() == 2);
    TEST_ASSERT(count_events(ON) == 2);
    first = find_event(ON, 0);
    second = find_event(ON, first + 1);
    TEST_ASSERT(second < event_count);
    TEST_ASSERT(events[second].at_ms - events[first].at_ms >= STARTUP_BURN_MS + STARTUP_WEEKLY_MS);
    TEST_ASSERT(events[second].at_ms - events[first].at_ms <= STARTUP_BURN_MS + STARTUP_WEEKLY_MS + SLOP_MS);
    require_media(WAITING_FOR_UPLINK, 7, STARTUP_WEEKLY_MS);

    /* Saturated: the count stays at seven and the weekly interval repeats. */
    scenario(WAIT_POWER_ON, 7, true, 0);
    stop_at = STARTUP_WEEKLY_MS + 2u * STARTUP_BURN_MS + SLOP_MS;
    TEST_ASSERT(model_run() == 2);
    first = find_event(ON, 0);
    second = find_event(ON, first + 1);
    TEST_ASSERT(second < event_count);
    TEST_ASSERT(events[second].at_ms - events[first].at_ms >= STARTUP_BURN_MS + STARTUP_WEEKLY_MS);
    require_media(WAITING_FOR_UPLINK, 7, STARTUP_WEEKLY_MS);

    /* First attempt: two days, then the count is two. */
    scenario(WAIT_POWER_ON, 0, true, 0);
    stop_at = STARTUP_RETRY_MS + 2u * STARTUP_BURN_MS + SLOP_MS;
    TEST_ASSERT(model_run() == 2);
    first = find_event(ON, 0);
    second = find_event(ON, first + 1);
    TEST_ASSERT(second < event_count);
    TEST_ASSERT(events[second].at_ms - events[first].at_ms >= STARTUP_BURN_MS + STARTUP_RETRY_MS);
    require_media(WAITING_FOR_UPLINK, 2, STARTUP_RETRY_MS);
}

void test_startup_pending_start_wins_without_a_burn(void) {
    /* A START waiting during the cooldown skips the pending retry. */
    scenario(WAITING_FOR_UPLINK, 3, true, STARTUP_RETRY_MS);
    input("START");
    input("ACK");
    TEST_ASSERT(model_run() == 0);
    TEST_ASSERT(count_events(ON) == 0);
    TEST_ASSERT(find_event(HELLO, 0) < event_count);
    require_media(MAIN_OPS, 3, 0);

    /* A retry that receives START after the reservation was saved commands OFF and
     * enters the saved handshake state, still without ON. */
    scenario(WAIT_POWER_ON, 3, true, 0);
    start_available_at_pull = 2;
    input("START");
    input("ACK");
    TEST_ASSERT(model_run() == 0);
    TEST_ASSERT(count_events(ON) == 0);
    TEST_ASSERT(count_events(OFF) >= 2);
    TEST_ASSERT(find_event(HELLO, 0) < event_count);
    require_media(MAIN_OPS, 4, 0);

    /* The first normal burn still precedes listening: a START already queued for
     * the very first eligible boot must not skip that burn (SPEC item 3). */
    scenario(WAIT_POWER_ON, 0, true, 0);
    start_available_at_pull = 1;
    input("START");
    input("ACK");
    TEST_ASSERT(model_run() == 0);
    TEST_ASSERT(count_events(ON) == 1);
    require_media(MAIN_OPS, 1, 0);
}

void test_startup_handshake_requires_ack_and_ready_is_best_effort(void) {
    /* The ground ACK gates success; HELLO is retried on the response timeout. */
    scenario(SAT_TO_GROUND_1, 1, true, 0);
    input_at("ACK", 3, 2u * STARTUP_RESPONSE_MS + 10u * STARTUP_POLL_MS);
    TEST_ASSERT(model_run() == 0);
    TEST_ASSERT(count_events(ON) == 0);
    TEST_ASSERT(count_events(HELLO) >= 3);
    TEST_ASSERT(count_events(READY) == 1);
    size_t first = find_event(HELLO, 0), second = find_event(HELLO, first + 1);
    TEST_ASSERT(second < event_count);
    TEST_ASSERT(events[second].at_ms - events[first].at_ms >= STARTUP_HELLO_RETRY_MS);
    require_media(MAIN_OPS, 1, 0);

    /* Only the exact tokens are accepted; near misses are ignored. */
    scenario(SAT_TO_GROUND_1, 1, true, 0);
    input("start");
    input("START\n");
    input("STAR");
    input_at("START", 6, 0);
    input_at("ACK", 4, 0);
    input("AC");
    input("HELLO");
    input("ACK");
    TEST_ASSERT(model_run() == 0);
    TEST_ASSERT(count_events(HELLO) == 1);
    TEST_ASSERT(count_events(READY) == 1);
    require_media(MAIN_OPS, 1, 0);

    /* A completed boot never repeats deployment or the handshake. */
    unsigned on_before = on_count;
    model_reboot();
    TEST_ASSERT(model_run() == 0);
    TEST_ASSERT(on_count == on_before);
    TEST_ASSERT(count_events(HELLO) == 0);
    TEST_ASSERT(count_events(READY) == 0);
    require_media(MAIN_OPS, 1, 0);
}

void test_startup_reset_recovery_never_replays_a_commitment(void) {
    /* Reset after the handshake entry: the reboot resumes at the handshake. */
    scenario(SAT_TO_GROUND_1, 1, true, 0);
    stop_at = 1;
    TEST_ASSERT(model_run() == 2);
    TEST_ASSERT(count_events(ON) == 0);
    model_reboot();
    input("ACK");
    TEST_ASSERT(model_run() == 0);
    TEST_ASSERT(count_events(ON) == 0);
    require_media(MAIN_OPS, 1, 0);

    /* Reset while the pre-ON save is torn: no ON is authorized, and the
     * uncommitted decision is retried on the next boot. */
    scenario(WAIT_POWER_ON, 0, true, 0);
    cut_budget = 5;
    TEST_ASSERT(model_run() == 1);
    TEST_ASSERT(count_events(ON) == 0);
    size_t reset = find_event(RESET, 0);
    TEST_ASSERT(reset > 0 && reset < event_count);
    TEST_ASSERT(events[reset - 1].action == OFF);
    cut_budget = -1;
    power_lost = false;
    require_media(WAIT_POWER_ON, 0, 0);
    model_reboot();
    start_available_at_pull = 2; /* the uplink arrives after the retried burn */
    input("START");
    input("ACK");
    TEST_ASSERT(model_run() == 0);
    TEST_ASSERT(count_events(ON) == 1);
    require_media(MAIN_OPS, 1, 0);

    /* Reset after the reservation committed: the consumed attempt waits out the
     * full saved interval and is never replayed immediately. */
    scenario(WAITING_FOR_UPLINK, 1, true, STARTUP_RETRY_MS);
    stop_at = STARTUP_RETRY_MS - 1000;
    TEST_ASSERT(model_run() == 2);
    TEST_ASSERT(count_events(ON) == 0);
    Ssp1Record pending;
    TEST_ASSERT(media_current(&pending));
    TEST_ASSERT(pending.state == WAITING_FOR_UPLINK && pending.attempts == 1);
    TEST_ASSERT(pending.remaining <= STARTUP_RETRY_MS);
    model_reboot();
    stop_at = 1000;
    TEST_ASSERT(model_run() == 2);
    TEST_ASSERT(count_events(ON) == 0);
}

void test_startup_storage_faults_fail_closed_with_off_then_reset(void) {
    /* No usable record at all. */
    scenario(INITIAL_CHECKS, 0, true, 0);
    memset(fram + SSP1_ADDRESS, 0xFF, SSP1_IMAGE_SIZE);
    TEST_ASSERT(model_run() == 1);
    TEST_ASSERT(count_events(ON) == 0);
    TEST_ASSERT(count_events(RETURNED) == 0);
    size_t reset = find_event(RESET, 0);
    TEST_ASSERT(reset > 0 && events[reset - 1].action == OFF);

    /* Equal sequences with different payloads are ambiguous. */
    scenario(INITIAL_CHECKS, 0, true, 1000);
    ssp1_build(fram + SSP1_ADDRESS + SSP1_SLOT_SIZE, 1, INITIAL_CHECKS, 0, true, 2000);
    TEST_ASSERT(model_run() == 1);
    TEST_ASSERT(count_events(ON) == 0);

    /* Unreadable media. */
    scenario(INITIAL_CHECKS, 0, true, 1000);
    fail_read = true;
    TEST_ASSERT(model_run() == 1);
    TEST_ASSERT(count_events(ON) == 0);

    /* A save that cannot be verified never authorizes ON. */
    scenario(WAIT_POWER_ON, 0, true, 0);
    fail_write = true;
    TEST_ASSERT(model_run() == 1);
    TEST_ASSERT(count_events(ON) == 0);
    reset = find_event(RESET, 0);
    TEST_ASSERT(reset > 0 && events[reset - 1].action == OFF);

    scenario(WAIT_POWER_ON, 0, true, 0);
    fail_wren = true;
    TEST_ASSERT(model_run() == 1);
    TEST_ASSERT(count_events(ON) == 0);

    scenario(WAIT_POWER_ON, 0, true, 0);
    fail_wrdi = true;
    TEST_ASSERT(model_run() == 1);
    TEST_ASSERT(count_events(ON) == 0);

    scenario(WAIT_POWER_ON, 0, true, 0);
    ignore_writes = true;
    TEST_ASSERT(model_run() == 1);
    TEST_ASSERT(count_events(ON) == 0);
}

void test_startup_wait_counts_elapsed_time_once(void) {
    /* Polling costs real time (radio RX with CRC retries). Every counted interval
     * must be subtracted exactly once, so a saved remaining time always matches
     * the real time that passed since the wait opened. */
    const uint32_t open_remaining = 300000;
    scenario(WAITING_FOR_UPLINK, 1, true, open_remaining);
    poll_cost_ms = 2500;
    stop_at = open_remaining + 2u * STARTUP_BURN_MS + SLOP_MS;
    TEST_ASSERT(model_run() == 2);
    TEST_ASSERT(count_events(ON) == 1);
    size_t on = find_event(ON, 0);
    TEST_ASSERT(on < event_count);
    /* No early expiry: the attempt is not reached before the saved time passed. */
    TEST_ASSERT(events[on].at_ms >= open_remaining);

    unsigned checkpoints = 0;
    for (size_t i = 0; i < save_count; ++i) {
        /* Checkpoints of this wait only: the later reservation stores the next interval. */
        if (saves[i].state != WAITING_FOR_UPLINK || saves[i].at_ms > open_remaining) continue;
        ++checkpoints;
        uint64_t expected = open_remaining > saves[i].at_ms ? open_remaining - saves[i].at_ms : 0;
        if (saves[i].remaining > expected + 1 || expected > saves[i].remaining + 1) {
            printf("  checkpoint at %" PRIu64 " ms saved remaining %" PRIu32
                   ", real remaining %" PRIu64 "\n",
                   saves[i].at_ms, saves[i].remaining, expected);
            TEST_ASSERT(false);
            return;
        }
    }
    TEST_ASSERT(checkpoints >= 4);
}

void test_startup_listening_polls_within_a_second(void) {
    /* START must not wait behind a long blind sleep: the wait is polled at least
     * once a second, so a command that arrives mid-cooldown is taken promptly. */
    scenario(WAITING_FOR_UPLINK, 1, true, STARTUP_RETRY_MS);
    input_at("START", 5, 2000);
    input("ACK");
    TEST_ASSERT(model_run() == 0);
    size_t hello = find_event(HELLO, 0);
    TEST_ASSERT(hello < event_count);
    TEST_ASSERT(events[hello].at_ms >= 2000);
    TEST_ASSERT(events[hello].at_ms <= 2000 + STARTUP_POLL_MS + SLOP_MS);
    TEST_ASSERT(count_events(ON) == 0);
    require_media(MAIN_OPS, 1, 0);

    /* The minute FRAM cadence stays separate from the 1 s polling cadence. */
    scenario(WAITING_FOR_UPLINK, 1, true, 180000);
    stop_at = 130000;
    TEST_ASSERT(model_run() == 2);
    TEST_ASSERT(save_count >= 2);
    for (size_t i = 1; i < save_count; ++i) {
        TEST_ASSERT(saves[i].at_ms - saves[i - 1].at_ms <= STARTUP_CHECKPOINT_MS);
        TEST_ASSERT(saves[i].at_ms - saves[i - 1].at_ms >= STARTUP_CHECKPOINT_MS - STARTUP_POLL_MS);
    }
}

void test_startup_unarmed_record_stays_inhibited(void) {
    largest_delay_ms = 0;
    /* Deliberate inhibition never enters the countdown, even beyond 30 minutes. */
    scenario(INITIAL_CHECKS, 0, false, STARTUP_INHIBIT_MS);
    stop_at = STARTUP_INHIBIT_MS + STARTUP_CHECKPOINT_MS;
    TEST_ASSERT(model_run() == 2);
    TEST_ASSERT(count_events(ON) == 0);
    TEST_ASSERT(count_events(RETURNED) == 0);
    TEST_ASSERT(count_events(HELLO) == 0);
    TEST_ASSERT(reset_count == 0);
    TEST_ASSERT(write_count == 0);
    TEST_ASSERT(delay_calls > 200); /* bounded awake pacing, not a stalled loop */
    TEST_ASSERT(largest_delay_ms <= STARTUP_POLL_MS); /* no initial countdown wait */
    Ssp1Record parked;
    TEST_ASSERT(media_current(&parked));
    TEST_ASSERT(parked.state == INITIAL_CHECKS);
    TEST_ASSERT(parked.flags == 0x00u);
    TEST_ASSERT(parked.remaining == STARTUP_INHIBIT_MS);

    /* Same rule when nothing is left to count down. */
    scenario(INITIAL_CHECKS, 0, false, 0);
    stop_at = 10000;
    TEST_ASSERT(model_run() == 2);
    TEST_ASSERT(count_events(ON) == 0);
    TEST_ASSERT(write_count == 0);
    TEST_ASSERT(reset_count == 0);
    require_media(INITIAL_CHECKS, 0, 0);
}

#ifdef STARTUP_TEST_DEFAULT_BATTERY
void test_startup_production_default_battery_never_deploys(void) {
    /* STARTUP_BATTERY_ELIGIBLE() is the production default (false): the attempt
     * stays pending with the pyro OFF and nothing is rewritten. */
    scenario(WAIT_POWER_ON, 0, true, 0);
    stop_at = 10000;
    TEST_ASSERT(model_run() == 2);
    TEST_ASSERT(count_events(ON) == 0);
    TEST_ASSERT(write_count == 0);
    TEST_ASSERT(reset_count == 0);
    require_media(WAIT_POWER_ON, 0, 0);

    /* The inhibit still runs and is recorded, then power-on stays inhibited. */
    scenario(INITIAL_CHECKS, 0, true, STARTUP_INHIBIT_MS);
    stop_at = STARTUP_INHIBIT_MS + 10000;
    TEST_ASSERT(model_run() == 2);
    TEST_ASSERT(count_events(ON) == 0);
    require_media(WAIT_POWER_ON, 0, 0);
}
#endif

int main(void) {
#ifdef STARTUP_TEST_DEFAULT_BATTERY
    TEST_SUITE("actual Startup with the production battery default");
    RUN_TEST(test_startup_production_default_battery_never_deploys);
    TEST_RESULTS();
#else
    TEST_SUITE("actual Startup over modeled peripherals");
    RUN_TEST(test_startup_inhibit_saves_attempt_before_on_and_completes);
    RUN_TEST(test_startup_resume_never_credits_unknown_time);
    RUN_TEST(test_startup_interval_boundaries_and_attempt_saturation);
    RUN_TEST(test_startup_pending_start_wins_without_a_burn);
    RUN_TEST(test_startup_wait_counts_elapsed_time_once);
    RUN_TEST(test_startup_listening_polls_within_a_second);
    RUN_TEST(test_startup_handshake_requires_ack_and_ready_is_best_effort);
    RUN_TEST(test_startup_reset_recovery_never_replays_a_commitment);
    RUN_TEST(test_startup_storage_faults_fail_closed_with_off_then_reset);
    RUN_TEST(test_startup_unarmed_record_stays_inhibited);
    TEST_RESULTS();
#endif
}
