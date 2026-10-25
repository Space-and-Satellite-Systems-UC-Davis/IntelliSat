#define _POSIX_C_SOURCE 200809L
#include "framework.h"
#include <inttypes.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include "RTC/rtc.h" /* Fixture observes unchanged legacy backup registers. */
#include "../startup.h" /* Actual blocking controller, included exactly once. */
#include "../startup_persistence.h"
#include "peripherals/FRAM/MB85RS256B.h"
#include "startup_fixtures.h"
/* Execute the small existing PDB initializer against register/line boundary models. */
#define pdb_init startup_test_real_pdb_init
#define pdb_pyro_on startup_test_real_pyro_on
#define pdb_pyro_off startup_test_real_pyro_off
#define pdb_mgt_on startup_test_real_mgt_on
#define pdb_mgt_off startup_test_real_mgt_off
#define pdb_hdd_on startup_test_real_hdd_on
#define pdb_hdd_off startup_test_real_hdd_off
#include "../../peripherals/PDB/power_distribution.c"
#undef pdb_init
#undef pdb_pyro_on
#undef pdb_pyro_off
#undef pdb_mgt_on
#undef pdb_mgt_off
#undef pdb_hdd_on
#undef pdb_hdd_off
GPIO_TypeDef startup_test_gpio_a, startup_test_gpio_c;
static unsigned off_latch_mode;
void gpio_low(GPIO_TypeDef *port, int pin) {
    off_latch_mode = (port->MODER >> (pin * 2)) & 3;
    port->ODR &= ~(1u << pin);
}
void gpio_high(GPIO_TypeDef *port, int pin) { port->ODR |= 1u << pin; }
#define N(a) (sizeof(a) / sizeof((a)[0]))
typedef enum { INIT, OFF, AWAKE_WAIT, ON, RX, HELLO, READY, DOWNLINK, RETURNED } Action;
typedef struct { Action action; uint64_t rtc, awake; uint8_t phase, count; } Event;
typedef struct { const char *bytes; size_t size; uint64_t not_before; } Input;
StartupTestRTC startup_test_rtc;
static uint8_t fram[32768];
static uint64_t rtc_ms, awake_ms, awake_waited_ms, early_wait_ms, stop_at, battery_at;
static uint32_t source, revision;
static bool retained, clock_valid, battery, pyro, enabled, fail_read, fail_write, fail_wrdi;
static Event events[4096];
static size_t event_count, input_pos, input_count;
static Input inputs[64];
static unsigned pulls, confirms, retained_reads, awake_waits, hello_failures, ready_failures;
static unsigned confirm_failures, fail_confirm_at, confirm_rejections;
static bool confirmation_ok, change_revision_on_confirmation;
static uint64_t last_rejection_awake;
static unsigned downlink_failures, revision_reads, change_revision_at;
static unsigned discontinuity_wait, invalid_wait, backwards_wait, source_wait;
static unsigned write_count, checkpoint_count;
static uint64_t last_checkpoint, max_checkpoint_gap, restore_clock_at;
static bool reset_on_phase, noop_write, corrupt_readback;
static StPhase reset_phase;
static int fail_write_phase;
static bool fail_ready_downlink, last_push_ready;
static jmp_buf reset_env;
static bool jumping;
static Action stop_action;
static unsigned stop_occurrence, action_occurrences[RETURNED + 1];
static int child_pipe = -1;
static unsigned budget;
static void require(bool condition, const char *why) {
    if (!condition) { fprintf(stderr, "Harness: %s\n", why); exit(2); }
}
static void boundary(void) { require(budget-- > 0, "unexpected loop"); }
static void event(Action a) {
    boundary();
    require(event_count < N(events), "event capacity");
    events[event_count++] = (Event){a, rtc_ms, awake_ms, fram[80], fram[81]};
    if (child_pipe >= 0) { char b = a == ON ? 'N' : a == RETURNED ? 'R' : a == OFF ? 'F' : '.'; (void)write(child_pipe, &b, 1); }
    if (jumping && a == stop_action && ++action_occurrences[a] == stop_occurrence) longjmp(reset_env, 1);
}
void pdb_init(void) { event(INIT); }
void pdb_pyro_on(void) {
    require(fram[80] == ST_PHASE_RESERVED && !memcmp(fram + 8, fram + 104, 96), "ON before mirrored reservation");
    pyro = true; event(ON);
}
void pdb_pyro_off(void) { pyro = false; event(OFF); }
bool startup_test_battery_eligible(void) { return battery && rtc_ms >= battery_at; }
uint64_t getSysTime(void) { return awake_ms; }
void radio_init(void) { require(!pyro, "UART init while ON"); }
void delay_ms(uint64_t ms) {
    boundary();
    /* Only a confirmed, still-inhibited wait counts; no modeled WFI or RTC IRQ. */
    bool waiting = confirmation_ok && !pyro && fram[80] == ST_PHASE_INHIBIT;
    if (waiting) {
        require(ms > 0 && ms <= STARTUP_CHECKPOINT_MS, "awake wait must stay checkpoint bounded");
        event(AWAKE_WAIT); ++awake_waits;
        confirmation_ok = false;
        if (early_wait_ms && ms > early_wait_ms) ms = early_wait_ms;
        awake_waited_ms += ms;
    }
    awake_ms += ms; rtc_ms += ms;
    if (waiting) {
        if (awake_waits == discontinuity_wait) { ++revision; rtc_ms += 900000000; }
        if (awake_waits == invalid_wait) clock_valid = false;
        if (awake_waits == backwards_wait) rtc_ms = 1000;
        if (awake_waits == source_wait) ++source;
    }
    if (restore_clock_at && rtc_ms >= restore_clock_at) clock_valid = true;
    if (jumping && stop_at && rtc_ms >= stop_at) longjmp(reset_env, 1);
}
bool PWR_enterLPSleepMode(uint16_t seconds) {
    (void)seconds;
    require(false, "Startup must not enter RTC-backed low-power sleep");
    return false;
}
/* Startup-private clock boundary substitute: the real module is exercised by
 * test_startup_clock.c and the real-adapter tracers in this directory. */
static StClock invalid_clock(void) {
    return (StClock){.epoch_ms = 0, .source = 0, .revision = 0, .valid = false};
}
StClock startup_clock_sample(void) {
    uint32_t before = revision;
    if (++revision_reads == change_revision_at) ++revision; /* modeled domain change mid-sample */
    StClock clock = {.epoch_ms = rtc_ms, .source = source, .revision = revision,
                     .valid = clock_valid && revision == before};
    return clock;
}
StClock startup_clock_anchor(void) {
    StClock first = startup_clock_sample();
    if (!first.valid) return invalid_clock();
    /* The anchor consumes one bounded witness step of real time. */
    rtc_ms += STARTUP_CLOCK_ANCHOR_STEP_MS;
    awake_ms += STARTUP_CLOCK_ANCHOR_STEP_MS;
    StClock later = startup_clock_sample();
    if (!later.valid || later.source != first.source || later.revision != first.revision ||
        later.epoch_ms < first.epoch_ms + STARTUP_CLOCK_ANCHOR_STEP_MS) return invalid_clock();
    return later;
}
bool startup_clock_retained(void) { ++retained_reads; return retained; }
bool startup_clock_confirm_wait(uint32_t expected_source, uint32_t expected_revision) {
    require(checkpoint_count > 0, "continuity confirmation before durable progress");
    require(expected_source == source && expected_revision == revision, "wrong confirmation source/revision");
    ++confirms;
    if (confirms >= fail_confirm_at && confirm_failures) {
        --confirm_failures; ++confirm_rejections;
        if (change_revision_on_confirmation) ++revision;
        last_rejection_awake = awake_ms;
        confirmation_ok = false;
        return false;
    }
    confirmation_ok = clock_valid;
    return confirmation_ok;
}
uint32_t rtc_getUnixTime(void) { return (uint32_t)(rtc_ms / 1000); }
void rtc_writeToBKPNumber(uint32_t bits, uint32_t bkp) { (void)bits; (void)bkp; require(false, "legacy mission BKP use"); }
RadioPacket radio_force_pull(uint8_t *chunk) {
    boundary(); require(!pyro, "radio while ON"); ++pulls;
    if (input_pos == input_count || rtc_ms < inputs[input_pos].not_before) return (RadioPacket){0, 0};
    Input p = inputs[input_pos++];
    if (p.size && p.size <= MAX_PAYLOAD_BYTES && p.bytes) memcpy(chunk, p.bytes, p.size);
    event(RX);
    return (RadioPacket){p.size, 'u'};
}
bool radio_push(uint8_t *chunk, size_t size) {
    require(!pyro, "TX while ON");
    require(fram[80] == ST_PHASE_RETIRED && !memcmp(fram + 8, fram + 104, 96), "handshake before mirrored retirement");
    bool hello = size == 5 && !memcmp(chunk, "HELLO", 5);
    bool ready = size == 5 && !memcmp(chunk, "READY", 5);
    /* The initial red tracer reports legacy HS1/HS3 without accepting them. */
    last_push_ready = ready;
    event(hello ? HELLO : READY);
    require(hello || ready, "unexpected handshake bytes (legacy HS1/HS3)");
    if (hello && hello_failures) { --hello_failures; return false; }
    if (ready && ready_failures) { --ready_failures; return false; }
    return true;
}
bool radio_downlink(uint8_t *chunk, size_t count) {
    (void)chunk; require(count == 1, "short message is one CHUNK_LENGTH chunk");
    event(DOWNLINK);
    if (last_push_ready && fail_ready_downlink) return false;
    if (downlink_failures) { --downlink_failures; return false; }
    return true;
}
int printMsg(const char *format, ...) { (void)format; boundary(); return 0; }
bool FRAM_readData(uint16_t address, uint8_t *out, uint16_t length) {
    if (fail_read) return false;
    require((size_t)address + length <= sizeof(fram), "FRAM read bounds");
    memcpy(out, fram + address, length);
    if (corrupt_readback && write_count && length < 320) out[0] ^= 1;
    if (jumping && reset_on_phase && length == 320 && fram[80] == reset_phase &&
        memcmp(fram + 8, fram + 104, 96) == 0) longjmp(reset_env, 1);
    return true;
}
bool FRAM_writeEnable(void) { enabled = !fail_write; return enabled; }
bool FRAM_writeDisable(void) { enabled = false; return !fail_wrdi; }
bool FRAM_writeData(uint16_t address, uint8_t *in, uint16_t length) {
    require(!pyro, "storage write while ON");
    require(address >= 8 && (size_t)address + length <= 328, "startup FRAM bounds");
    if (fail_write || !enabled ||
        (length == 92 && (address == 12 || address == 108) && in[68] == fail_write_phase)) return false;
    if (!noop_write) memcpy(fram + address, in, length);
    ++write_count;
    if ((address == 200 || address == 264) && length == 4 && !memcmp(in, "STP3", 4)) {
        ++checkpoint_count;
        if (last_checkpoint && rtc_ms - last_checkpoint > max_checkpoint_gap) max_checkpoint_gap = rtc_ms - last_checkpoint;
        last_checkpoint = rtc_ms;
    }
    return true;
}
static void scenario(StPhase phase, unsigned count, uint64_t elapsed) {
    memset(fram, 0xa5, sizeof(fram));
    fixture_seed(fram, phase, count, elapsed, 100000, true);
    startup_test_rtc = (StartupTestRTC){.BKP0R = 11, .BKP1R = 22, .BKP2R = 33,
                                        .BKP3R = 44, .BKP4R = 55, .BKP5R = 66, .BKP6R = 77};
    rtc_ms = 100000; awake_ms = awake_waited_ms = 0;
    source = revision = 1; retained = clock_valid = battery = true;
    pyro = enabled = fail_read = fail_write = fail_wrdi = false;
    early_wait_ms = battery_at = stop_at = restore_clock_at = 0;
    reset_on_phase = noop_write = corrupt_readback = false;
    fail_write_phase = -1; fail_ready_downlink = last_push_ready = false;
    event_count = input_pos = input_count = 0;
    pulls = confirms = retained_reads = awake_waits = hello_failures = ready_failures = 0;
    confirm_failures = fail_confirm_at = confirm_rejections = 0;
    confirmation_ok = change_revision_on_confirmation = false;
    last_rejection_awake = 0;
    downlink_failures = revision_reads = change_revision_at = 0;
    discontinuity_wait = invalid_wait = backwards_wait = source_wait = 0;
    write_count = checkpoint_count = 0; last_checkpoint = max_checkpoint_gap = 0;
    memset(action_occurrences, 0, sizeof(action_occurrences));
    jumping = false; child_pipe = -1; budget = 100000000;
}
static void input(const char *s) { inputs[input_count++] = (Input){s, s ? strlen(s) : 0, 0}; }
static void handshake(void) { input("START"); input("ACK"); }
/* The firmware boot chain is init_init() then Startup(): init_init() owns the PDB
 * safe state (clocked ports, pdb_init(), explicit OFF) and runs on every boot, so
 * each modeled boot replays that prefix. Startup() itself must not re-init the PDB,
 * and its radio_init() refuses to run while the actuator is still commanded ON.
 */
static void model_boot_entry(void) {
    pdb_init();
    pdb_pyro_off();
}
static void run(void) {
    model_boot_entry();
    Startup();
    require(fram[80] == ST_PHASE_COMPLETE && !memcmp(fram + 8, fram + 104, 96), "return before mirrored completion");
    event(RETURNED);
}
static size_t find(Action a, size_t from) {
    for (size_t i = from; i < event_count; ++i) if (events[i].action == a) return i;
    return event_count;
}
static unsigned count(Action a) {
    unsigned n = 0; for (size_t i = 0; i < event_count; ++i) n += events[i].action == a; return n;
}
static void order(const Action *a, size_t n) {
    size_t pos = 0;
    for (size_t i = 0; i < n; ++i) { pos = find(a[i], pos); TEST_ASSERT(pos < event_count); ++pos; }
}
static void saved(StPhase phase, unsigned fast_count) {
    StSnapshot s;
    TEST_ASSERT(st_load(&s) == ST_OK);
    TEST_ASSERT(s.control.phase == phase);
    TEST_ASSERT(s.control.fast_count == fast_count);
    for (unsigned i = 0; i < 8; ++i) TEST_ASSERT(fram[i] == 0xa5);
    TEST_ASSERT(RTC->BKP0R == 11 && RTC->BKP1R == 22 && RTC->BKP2R == 33 &&
                RTC->BKP3R == 44 && RTC->BKP4R == 55 && RTC->BKP5R == 66 && RTC->BKP6R == 77);
}
static void test_startup_normal_wait_command_handshake_return(void) {
    scenario(ST_PHASE_PREPARED, 0, 0); handshake(); run();
    for (size_t i = 0; i < event_count; ++i) {
        printf("  action=%u rtc_ms=%" PRIu64 " awake_ms=%" PRIu64 " durable_phase=%u count=%u\n",
               events[i].action, events[i].rtc, events[i].awake, events[i].phase, events[i].count);
    }
    const Action a[] = {INIT, OFF, AWAKE_WAIT, ON, OFF, RX, HELLO, DOWNLINK, RX, READY, DOWNLINK, RETURNED};
    order(a, N(a));
    size_t on = find(ON, 0), off = find(OFF, on + 1);
    TEST_ASSERT(on < event_count && events[on].rtc >= 1900000);
    TEST_ASSERT(off < event_count && events[off].awake - events[on].awake == 5000);
    TEST_ASSERT(count(ON) == 1);
    /* The entry witness already accrued 5 ms before awake waiting began. */
    TEST_ASSERT(awake_waited_ms + STARTUP_CLOCK_ANCHOR_STEP_MS == 1800000);
    TEST_ASSERT(awake_waits == 30);
    TEST_ASSERT(rtc_ms - 100000 == awake_ms);
    TEST_ASSERT(confirms >= 30 && retained_reads == 1);
    TEST_ASSERT(max_checkpoint_gap <= 60000);
    TEST_ASSERT(events[on].phase == ST_PHASE_RESERVED && events[on].count == 1);
    saved(ST_PHASE_COMPLETE, 1);
}
/* Host-only reset injection abandons the invocation, never changes production control flow. */
static void pause_startup(Action action, unsigned occurrence, uint64_t deadline) {
    jumping = true; stop_action = action; stop_occurrence = occurrence; stop_at = deadline;
    if (!setjmp(reset_env)) { run(); require(false, "expected modeled reset/observation boundary"); }
    jumping = false;
}
static void model_reboot(void) {
    awake_ms = 0; event_count = input_pos = input_count = 0;
    memset(action_occurrences, 0, sizeof(action_occurrences));
    reset_on_phase = false; stop_at = 0; jumping = false;
    /* Preserve FRAM, clock and modeled actuator level until the next OFF command. */
}
static void test_startup_pdb_clears_latch_before_output_mode(void) {
    startup_test_gpio_c = (GPIO_TypeDef){UINT32_MAX, UINT32_MAX};
    startup_test_gpio_a = (GPIO_TypeDef){UINT32_MAX, UINT32_MAX};
    off_latch_mode = 99;
    startup_test_real_pdb_init();
    TEST_ASSERT(off_latch_mode == 3);
    TEST_ASSERT((startup_test_gpio_c.ODR & (1u << 9)) == 0);
    TEST_ASSERT(((startup_test_gpio_c.MODER >> 18) & 3) == 1);
}
static void test_startup_exact_and_short_awake_waits(void) {
    scenario(ST_PHASE_PREPARED, 0, 0); early_wait_ms = 13000; handshake(); run();
    TEST_ASSERT(awake_waited_ms + STARTUP_CLOCK_ANCHOR_STEP_MS == 1800000 && awake_waits > 30);
    TEST_ASSERT(events[find(ON, 0)].rtc >= 1900000);
    scenario(ST_PHASE_INHIBIT, 0, 1799999); retained = false; handshake(); run();
    TEST_ASSERT(awake_waited_ms == 1); /* Exact remaining millisecond, never early ON. */
    scenario(ST_PHASE_INHIBIT, 0, 0); rtc_ms += 1800000; handshake(); run();
    TEST_ASSERT(awake_waited_ms == 0 && count(ON) == 1);
}
static void test_startup_saved_progress_with_and_without_clock(void) {
    scenario(ST_PHASE_INHIBIT, 0, 1500000); retained = false; rtc_ms += 900000000;
    handshake(); run();
    TEST_ASSERT(awake_waited_ms == 300000 && retained_reads == 1);
    scenario(ST_PHASE_INHIBIT, 0, 1500000); rtc_ms += 300000; handshake(); run();
    TEST_ASSERT(awake_waited_ms == 0 && count(ON) == 1);
}
static void test_startup_clock_discontinuities_drop_unsaved_credit(void) {
    for (unsigned kind = 0; kind < 4; ++kind) {
        scenario(ST_PHASE_PREPARED, 0, 0);
        if (kind == 0) discontinuity_wait = 5;
        if (kind == 1) backwards_wait = 5;
        if (kind == 2) source_wait = 5;
        if (kind == 3) { invalid_wait = 5; restore_clock_at = 405000; }
        handshake(); run();
        TEST_ASSERT(awake_waited_ms + STARTUP_CLOCK_ANCHOR_STEP_MS >= 1860000);
        TEST_ASSERT(retained_reads == 1); /* Never re-credit the old clock domain. */
        TEST_ASSERT(count(ON) == 1 && count(RETURNED) == 1);
    }
    scenario(ST_PHASE_PREPARED, 0, 0); change_revision_at = 2; handshake(); run();
    TEST_ASSERT(events[find(ON, 0)].rtc >= 1901000); /* Torn first sample cannot anchor. */
}
static void test_startup_retry_torn_entry_clock_never_credits_old_domain(void) {
    scenario(ST_PHASE_RETRY_WAIT, 1, 0);
    rtc_ms += 172800000; change_revision_at = 2;
    pause_startup(RETURNED, 1, rtc_ms + 120000);
    TEST_ASSERT(count(ON) == 0);
    saved(ST_PHASE_RETRY_WAIT, 1);
}
static void test_startup_low_energy_and_default_false(void) {
    scenario(ST_PHASE_ELIGIBLE, 0, 0); battery = false;
    pause_startup(RETURNED, 1, rtc_ms + 120000);
    TEST_ASSERT(count(ON) == 0 && write_count == 0 && pulls == 0);
    saved(ST_PHASE_ELIGIBLE, 0);
    scenario(ST_PHASE_PREPARED, 0, 0); battery = false;
    pause_startup(RETURNED, 1, rtc_ms + 1860000);
    TEST_ASSERT(count(ON) == 0 && awake_waits == 30);
    saved(ST_PHASE_ELIGIBLE, 0);
    scenario(ST_PHASE_COMPLETE, 7, 0); battery = false; run();
    TEST_ASSERT(count(ON) == 0 && pulls == 0 && count(RETURNED) == 1);
    TEST_ASSERT(rtc_ms - 100000 <= 2000);
    saved(ST_PHASE_COMPLETE, 7);
}
static void test_startup_low_energy_retry_keeps_due_wait(void) {
    scenario(ST_PHASE_RETRY_WAIT, 6, 172770000); battery = false;
    pause_startup(RETURNED, 1, rtc_ms + 120000);
    TEST_ASSERT(count(ON) == 0 && checkpoint_count >= 1);
    saved(ST_PHASE_RETRY_WAIT, 6);
    StSnapshot s; TEST_ASSERT(st_load(&s) == ST_OK);
    TEST_ASSERT(s.has_progress && s.progress.elapsed_ms == 172800000);
    battery = true; model_reboot(); pause_startup(ON, 1, 0);
    TEST_ASSERT(events[find(ON, 0)].count == 7);
    model_reboot(); handshake(); run();
    TEST_ASSERT(count(ON) == 0); saved(ST_PHASE_COMPLETE, 7);
}
static void test_startup_seven_total_then_weekly_without_catchup(void) {
    scenario(ST_PHASE_ELIGIBLE, 0, 0);
    pause_startup(ON, 9, 0); /* Seven fast reservations, then two weekly reservations. */
    TEST_ASSERT(count(ON) == 9);
    size_t previous = find(ON, 0);
    for (unsigned n = 2; n <= 9; ++n) {
        size_t next = find(ON, previous + 1);
        uint64_t spacing = n <= 7 ? 172800000 : 604800000;
        TEST_ASSERT(events[next].rtc - events[previous].rtc >= spacing + 5000);
        TEST_ASSERT(events[next].rtc - events[previous].rtc < spacing + 10000);
        TEST_ASSERT(events[next].count == (n < 7 ? n : 7));
        previous = next;
    }
    TEST_ASSERT(max_checkpoint_gap <= 62000);
    model_reboot(); handshake(); run(); saved(ST_PHASE_COMPLETE, 7);
}
static void test_startup_pending_start_wins_due_retry_and_late_reservation(void) {
    for (unsigned stage = 0; stage < 3; ++stage) {
        scenario(ST_PHASE_RETRY_WAIT, 6, 172800000);
        for (unsigned i = 0; i < stage; ++i) input(NULL);
        handshake(); run();
        TEST_ASSERT(count(ON) == 0);
        size_t hello = find(HELLO, 0);
        TEST_ASSERT(hello < event_count && events[hello].phase == ST_PHASE_RETIRED);
        saved(ST_PHASE_COMPLETE, stage == 2 ? 7 : 6);
    }
}
static void test_startup_exact_packet_roles_reject_bad_and_duplicate_data(void) {
    scenario(ST_PHASE_RETRY_WAIT, 1, 0);
    input(NULL); inputs[input_count++] = (Input){NULL, (size_t)-1, 0};
    inputs[input_count++] = (Input){NULL, MAX_PAYLOAD_BYTES + 1, 0};
    input("start"); input("START\n"); input("STAR"); input("ACK"); input("HELLO");
    inputs[input_count++] = (Input){"START", 6, 0}; /* Trailing NUL is not the exact token. */
    input("START"); input("START"); input(NULL);
    inputs[input_count++] = (Input){NULL, (size_t)-1, 0};
    inputs[input_count++] = (Input){NULL, MAX_PAYLOAD_BYTES + 1, 0};
    input("HELLO"); input("ACK\n"); input("AC");
    inputs[input_count++] = (Input){"ACK", 4, 0};
    input("ACK");
    run();
    TEST_ASSERT(count(HELLO) == 1 && count(READY) == 1 && input_pos == input_count);
    TEST_ASSERT(count(ON) == 0);
    size_t hello = find(HELLO, 0);
    TEST_ASSERT(hello > 0 && events[hello].phase == ST_PHASE_RETIRED);
    saved(ST_PHASE_COMPLETE, 1);
}
static void test_startup_failed_hello_paced_and_local_downlink_required(void) {
    scenario(ST_PHASE_RETIRED, 1, 0); hello_failures = 2; input("ACK"); run();
    TEST_ASSERT(count(HELLO) == 3 && count(DOWNLINK) == 2);
    size_t a = find(HELLO, 0), b = find(HELLO, a + 1), c = find(HELLO, b + 1);
    TEST_ASSERT(events[b].awake - events[a].awake >= STARTUP_HELLO_RETRY_MS);
    TEST_ASSERT(events[c].awake - events[b].awake >= STARTUP_HELLO_RETRY_MS);
    TEST_ASSERT(count(ON) == 0); saved(ST_PHASE_COMPLETE, 1);
    scenario(ST_PHASE_RETIRED, 1, 0); downlink_failures = 1; input("ACK"); run();
    TEST_ASSERT(count(HELLO) == 2 && count(DOWNLINK) == 3);
    TEST_ASSERT(events[find(HELLO, find(HELLO, 0) + 1)].awake - events[find(HELLO, 0)].awake >= STARTUP_HELLO_RETRY_MS);
}
static void test_startup_lost_ack_timeout_restarts_hello(void) {
    scenario(ST_PHASE_RETIRED, 1, 0);
    inputs[input_count++] = (Input){"ACK", 3, rtc_ms + 2 * STARTUP_RESPONSE_MS + 10000};
    run();
    TEST_ASSERT(count(HELLO) >= 3 && count(READY) == 1 && count(ON) == 0);
    size_t a = find(HELLO, 0), b = find(HELLO, a + 1);
    TEST_ASSERT(events[b].awake - events[a].awake >= STARTUP_RESPONSE_MS);
    saved(ST_PHASE_COMPLETE, 1);
}
static void test_startup_ready_failure_does_not_gate_mission_after_ack(void) {
    scenario(ST_PHASE_RETIRED, 1, 0); ready_failures = 1; input("ACK"); run();
    TEST_ASSERT(count(READY) == 1 && count(RETURNED) == 1 && count(DOWNLINK) == 1);
    TEST_ASSERT(events[find(RETURNED, 0)].phase == ST_PHASE_COMPLETE);
    saved(ST_PHASE_COMPLETE, 1);
    scenario(ST_PHASE_RETIRED, 1, 0); fail_ready_downlink = true; input("ACK"); run();
    TEST_ASSERT(count(READY) == 1 && count(DOWNLINK) == 2 && count(RETURNED) == 1);
    saved(ST_PHASE_COMPLETE, 1);
}
static void test_startup_reset_reserved_on_off_never_replays_burn(void) {
    for (unsigned stage = 0; stage < 3; ++stage) {
        scenario(ST_PHASE_ELIGIBLE, 0, 0);
        if (stage == 0) { reset_on_phase = true; reset_phase = ST_PHASE_RESERVED; }
        pause_startup(stage == 2 ? OFF : ON, stage == 2 ? 2 : 1, 0);
        TEST_ASSERT(fram[80] == ST_PHASE_RESERVED);
        uint64_t recovery_time = rtc_ms + 900000000;
        rtc_ms = recovery_time; retained = false; model_reboot(); handshake(); run();
        TEST_ASSERT(count(ON) == 0 && count(OFF) >= 2);
        TEST_ASSERT(events[find(HELLO, 0)].phase == ST_PHASE_RETIRED);
        saved(ST_PHASE_COMPLETE, 1);
    }
    scenario(ST_PHASE_RESERVED, 6, 0); rtc_ms += 900000000; retained = false;
    uint64_t recovered_at = rtc_ms;
    pause_startup(RETURNED, 1, rtc_ms + 120000);
    TEST_ASSERT(count(ON) == 0);
    saved(ST_PHASE_RETRY_WAIT, 6);
    StSnapshot s; TEST_ASSERT(st_load(&s) == ST_OK);
    TEST_ASSERT(s.control.reference_ms >= recovered_at && s.control.duration_ms == 172800000);
    TEST_ASSERT(s.has_progress && s.progress.elapsed_ms <= 120000);
    model_reboot(); rtc_ms += 900000000;
    pause_startup(RETURNED, 1, rtc_ms + 120000);
    TEST_ASSERT(count(ON) == 0); /* Unknown off-time cannot finish the recovered wait. */
    saved(ST_PHASE_RETRY_WAIT, 6);
}
static void test_startup_reset_retirement_and_completion(void) {
    scenario(ST_PHASE_RETRY_WAIT, 1, 0); input("START");
    reset_on_phase = true; reset_phase = ST_PHASE_RETIRED;
    pause_startup(RETURNED, 1, 0);
    model_reboot(); input("ACK"); run();
    TEST_ASSERT(count(ON) == 0 && count(HELLO) == 1 && pulls == 2);
    saved(ST_PHASE_COMPLETE, 1);
    model_reboot(); run();
    TEST_ASSERT(count(ON) == 0 && count(HELLO) == 0 && count(READY) == 0);
    saved(ST_PHASE_COMPLETE, 1);
    scenario(ST_PHASE_RETIRED, 1, 0); input("ACK");
    pause_startup(READY, 1, 0);
    TEST_ASSERT(fram[80] == ST_PHASE_RETIRED);
    model_reboot(); input("ACK"); run();
    TEST_ASSERT(count(HELLO) == 1 && count(ON) == 0);
    saved(ST_PHASE_COMPLETE, 1);
    scenario(ST_PHASE_RETIRED, 1, 0); input("ACK");
    reset_on_phase = true; reset_phase = ST_PHASE_COMPLETE;
    pause_startup(RETURNED, 1, 0);
    model_reboot(); run(); TEST_ASSERT(count(HELLO) == 0 && count(ON) == 0);
}
static void test_startup_single_damaged_copy_repairs_before_on(void) {
    scenario(ST_PHASE_ELIGIBLE, 0, 0); fram[8] ^= 1; handshake(); run();
    TEST_ASSERT(count(ON) == 1);
    saved(ST_PHASE_COMPLETE, 1);
}
static void expect_fatal(void) {
    int fds[2]; require(pipe(fds) == 0, "pipe");
    pid_t child = fork(); require(child >= 0, "fork");
    if (!child) {
        close(fds[0]); child_pipe = fds[1];
        run(); _exit(3);
    }
    close(fds[1]);
    struct timespec delay = {0, 100000000}; nanosleep(&delay, NULL);
    int status = 0; pid_t result = waitpid(child, &status, WNOHANG);
    TEST_ASSERT(result == 0); /* The actual void Startup fatal halt must not return. */
    if (result == 0) { kill(child, SIGKILL); require(waitpid(child, &status, 0) == child, "reap"); }
    char data[4096]; ssize_t n = read(fds[0], data, sizeof(data)); close(fds[0]);
    TEST_ASSERT(n > 0 && memchr(data, 'F', (size_t)(n > 0 ? n : 0)) != NULL);
    TEST_ASSERT(n <= 0 || (memchr(data, 'N', (size_t)n) == NULL && memchr(data, 'R', (size_t)n) == NULL));
}
static void test_startup_fram_faults_and_inhibited_records_fail_closed(void) {
    for (unsigned kind = 0; kind < 8; ++kind) {
        scenario(ST_PHASE_PREPARED, 0, 0);
        if (kind == 0) memset(fram + 8, 0xff, 320);
        if (kind == 1) { fram[8] ^= 1; fram[104] ^= 1; }
        if (kind == 2) fixture_seed(fram, ST_PHASE_PREPARED, 0, 0, rtc_ms, false);
        if (kind == 3) fail_read = true;
        if (kind == 4) fail_write = true;
        if (kind == 5) fail_wrdi = true;
        if (kind == 6) noop_write = true;
        if (kind == 7) corrupt_readback = true;
        expect_fatal();
    }
    scenario(ST_PHASE_ELIGIBLE, 0, 0); fail_write_phase = ST_PHASE_RESERVED; expect_fatal();
    scenario(ST_PHASE_RESERVED, 1, 0); fail_write_phase = ST_PHASE_RETRY_WAIT; expect_fatal();
    scenario(ST_PHASE_RETRY_WAIT, 1, 0); input("START"); fail_write_phase = ST_PHASE_RETIRED; expect_fatal();
    scenario(ST_PHASE_RETIRED, 1, 0); input("ACK"); fail_write_phase = ST_PHASE_COMPLETE; expect_fatal();
}

static void test_startup_false_clock_confirmation_reanchors_before_wait_or_finish(void) {
    for (unsigned kind = 0; kind < 3; ++kind) {
        scenario(ST_PHASE_INHIBIT, 0, kind == 2 ? 1800000 : 1799999);
        fail_confirm_at = 1; confirm_failures = 1;
        change_revision_on_confirmation = kind == 1;
        handshake(); run();
        TEST_ASSERT(confirm_rejections == 1 && confirms >= 2);
        TEST_ASSERT(events[find(ON, 0)].awake >= last_rejection_awake + 1000);
        TEST_ASSERT(retained_reads == 1);
        saved(ST_PHASE_COMPLETE, 1);
    }
    scenario(ST_PHASE_INHIBIT, 0, 1800000);
    fail_confirm_at = 1; confirm_failures = 100;
    pause_startup(RETURNED, 1, rtc_ms + 5000);
    TEST_ASSERT(confirm_rejections >= 5 && count(ON) == 0 && awake_waits == 0);
    TEST_ASSERT(retained_reads == 1 && awake_ms >= 5000);
    saved(ST_PHASE_INHIBIT, 0);
}

int main(void) {
#ifdef STARTUP_TEST_DEFAULT_BATTERY
    RUN_TEST(test_startup_low_energy_and_default_false);
    TEST_RESULTS();
#endif
    TEST_SUITE("actual blocking Startup FRAM integration");
    RUN_TEST(test_startup_false_clock_confirmation_reanchors_before_wait_or_finish);
    RUN_TEST(test_startup_normal_wait_command_handshake_return);
    RUN_TEST(test_startup_pdb_clears_latch_before_output_mode);
    RUN_TEST(test_startup_exact_and_short_awake_waits);
    RUN_TEST(test_startup_saved_progress_with_and_without_clock);
    RUN_TEST(test_startup_clock_discontinuities_drop_unsaved_credit);
    RUN_TEST(test_startup_low_energy_and_default_false);
    RUN_TEST(test_startup_retry_torn_entry_clock_never_credits_old_domain);
    RUN_TEST(test_startup_low_energy_retry_keeps_due_wait);
    RUN_TEST(test_startup_seven_total_then_weekly_without_catchup);
    RUN_TEST(test_startup_pending_start_wins_due_retry_and_late_reservation);
    RUN_TEST(test_startup_exact_packet_roles_reject_bad_and_duplicate_data);
    RUN_TEST(test_startup_failed_hello_paced_and_local_downlink_required);
    RUN_TEST(test_startup_lost_ack_timeout_restarts_hello);
    RUN_TEST(test_startup_ready_failure_does_not_gate_mission_after_ack);
    RUN_TEST(test_startup_reset_reserved_on_off_never_replays_burn);
    RUN_TEST(test_startup_reset_retirement_and_completion);
    RUN_TEST(test_startup_single_damaged_copy_repairs_before_on);
    RUN_TEST(test_startup_fram_faults_and_inhibited_records_fail_closed);
    TEST_RESULTS();
}
