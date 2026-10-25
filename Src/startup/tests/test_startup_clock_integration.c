/*
 * Integration tracer: the real Startup interval clock inside the actual blocking
 * Startup() with the real persistence implementation.
 *
 * Peripheral boundaries (PDB, radio, FRAM, awake delay) are modeled. The clock,
 * persistence arithmetic and the deployment flow are the production sources.
 * Not an ARM build, LSE accuracy or electrical qualification.
 */
#include <setjmp.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "framework.h"
#include "../startup_persistence.h" /* StPhase/StSnapshot for the wire fixtures */
#include "startup_fixtures.h"
static void observe_awake_delay(uint64_t ms);
#define STARTUP_CLOCK_TEST_DELAY_HOOK(ms) observe_awake_delay(ms)
#include "clock_fixture.h"
#undef STARTUP_CLOCK_TEST_DELAY_HOOK
#include "peripherals/FRAM/MB85RS256B.h"
#include "../startup_clock.c"
#include "../startup_persistence.c"
#include "../startup_decisions.c"
#include "../startup.h"

#define STOP_RESET 1
#define STOP_DEADLINE 2

static uint8_t fram[32768];
static unsigned ons, offs, awake_waits, checkpoints;
static uint64_t on_epoch, last_awake_wait_ms;
static jmp_buf boot_env;
static bool armed;
static unsigned reset_after_checkpoint;
static uint64_t stop_at;
static uint64_t last_entry_epoch;
static const char *inputs[8];
static unsigned input_pos, input_count;
static unsigned long budget;

/* Fresh retry reference as persisted by st_after_off, and the OFF command it followed. */
static uint64_t off_epoch, fresh_reference, fresh_off_epoch;
static bool fresh_latched;

static uint64_t model_epoch(void) { return CLOCK_MODEL_EPOCH_MS + clock_model_ms; }

static uint64_t le64(const uint8_t *p) {
    uint64_t value = 0;
    for (unsigned i = 0; i < 8; ++i) value |= (uint64_t)p[i] << (i * 8);
    return value;
}

static void require(bool condition, const char *why) {
    if (!condition) { fprintf(stderr, "Tracer: %s\n", why); exit(2); }
}

/* A harness mistake must fail instead of spinning: every modeled long path is bounded. */
static void bound(void) {
    if (budget-- == 0U) { fprintf(stderr, "Tracer: work budget exhausted\n"); exit(3); }
}

static void check_deadline(void) {
    if (stop_at != 0U && model_epoch() >= stop_at) longjmp(boot_env, STOP_DEADLINE);
}

/* Observe the public delay boundary after modeled time advances. No RTC IRQ exists. */
static void observe_awake_delay(uint64_t ms) {
    bound();
    if (fram[80] == ST_PHASE_INHIBIT && ms > 0U) {
        require(ms <= STARTUP_CHECKPOINT_MS, "awake wait exceeds checkpoint interval");
        ++awake_waits;
        last_awake_wait_ms = ms;
    }
    check_deadline();
}

/* --- Peripheral boundary models --- */
void pdb_init(void) { bound(); }
void pdb_pyro_off(void) { bound(); ++offs; off_epoch = model_epoch(); }
void pdb_pyro_on(void) { bound(); ++ons; on_epoch = model_epoch(); }
void radio_init(void) { bound(); }
bool radio_push(uint8_t chunk[], size_t nbytes) { (void)chunk; bound(); return nbytes > 0; }
bool radio_downlink(uint8_t chunk[], size_t nchunks) { (void)chunk; bound(); return nchunks > 0; }
bool startup_test_battery_eligible(void) { bound(); return true; }
int printMsg(const char *format, ...) { (void)format; bound(); return 0; }
void gpio_low(GPIO_TypeDef *port, int pin) { (void)port; (void)pin; }
void gpio_high(GPIO_TypeDef *port, int pin) { (void)port; (void)pin; }

/* Scripted uplink traffic: START retires the wait, ACK completes the handshake. */
static void input(const char *packet) {
    require(input_count < 8, "input capacity");
    inputs[input_count++] = packet;
}
RadioPacket radio_force_pull(uint8_t chunk[]) {
    bound();
    check_deadline();
    if (input_pos >= input_count) return (RadioPacket){0, 0};
    const char *packet = inputs[input_pos++];
    size_t length = strlen(packet);
    memcpy(chunk, packet, length);
    return (RadioPacket){length, 'u'};
}

bool PWR_enterLPSleepMode(uint16_t seconds) {
    (void)seconds;
    require(false, "Startup called archived RTC-backed sleep");
    return false;
}

bool FRAM_readData(uint16_t address, uint8_t *out, uint16_t length) {
    bound();
    require((size_t)address + length <= sizeof(fram), "FRAM read bounds");
    memcpy(out, fram + address, length);
    return true;
}
bool FRAM_writeEnable(void) { return true; }
bool FRAM_writeDisable(void) { return true; }
bool FRAM_writeData(uint16_t address, uint8_t *in, uint16_t length) {
    bound();
    require(address >= 8 && (size_t)address + length <= 328, "startup FRAM bounds");
    memcpy(fram + address, in, length);
    if (address == 12 && length == 92 && in[68] == (uint8_t)ST_PHASE_RETRY_WAIT && !fresh_latched) {
        /* First retry-wait control payload (record bytes 4..95): the fresh reference
           st_after_off wrote, at record offset 56. */
        fresh_reference = le64(in + 52);
        fresh_off_epoch = off_epoch;
        fresh_latched = true;
    }
    if ((address == 200 || address == 264) && length == 4 && !memcmp(in, "STP3", 4)) {
        ++checkpoints;
        if (armed && checkpoints == reset_after_checkpoint) longjmp(boot_env, STOP_RESET);
    }
    return true;
}

/* --- Boot helpers --- */
static void boot_begin(uint32_t reset_flags) {
    (void)(RCC->CSR); /* apply any pending modeled RMVF clear first */
    RCC->CSR = reset_flags;
    ons = offs = awake_waits = checkpoints = 0;
    on_epoch = last_awake_wait_ms = 0;
    off_epoch = fresh_reference = fresh_off_epoch = 0;
    fresh_latched = false;
    input_pos = input_count = 0; /* each boot scripts only its own traffic */
    budget = 4000000UL;
    armed = false;
    stop_at = 0;
}

/* Power-on: the backup domain and its calendar are lost. */
static void cold_boot(uint32_t reset_flags) {
    clock_model_lost_domain();
    boot_begin(reset_flags);
}

/* Ordinary reset: the retained domain, calendar and time base carry over. */
static void warm_boot(uint32_t reset_flags) { boot_begin(reset_flags); }

static void enter_startup(void) {
    startup_clock_capture_boot_evidence();
    startup_clock_init();
    last_entry_epoch = model_epoch();
}

static int run_startup(void) {
    int result = setjmp(boot_env);
    if (result != 0) return result;
    Startup();
    return 0;
}

static void seed_state(StPhase phase, unsigned count, uint64_t elapsed, uint64_t reference) {
    memset(fram, 0, sizeof(fram));
    fixture_seed(fram, phase, count, elapsed, reference, true);
}

static StSnapshot load(void) {
    StSnapshot snapshot;
    require(st_load(&snapshot) == ST_OK || st_load(&snapshot) == ST_RECOVERABLE, "load");
    return snapshot;
}

static void test_cold_boot_seeds_the_fixed_interval_reference(void) {
    cold_boot(RCC_CSR_BORRSTF | RCC_CSR_PINRSTF);
    seed_state(ST_PHASE_PREPARED, 0, 0, 0);
    enter_startup();
    TEST_ASSERT(startup_clock_sample().valid);
    TEST_ASSERT(!startup_clock_retained()); /* power event is not yet acknowledged */
    /* The calendar is the fixed interval reference: 2001-01-01, Monday. */
    TEST_ASSERT(((startup_test_rtc.DR & RTC_DR_YU_Msk) >> RTC_DR_YU_Pos) == 1U);
    TEST_ASSERT(((startup_test_rtc.DR & RTC_DR_MU_Msk) >> RTC_DR_MU_Pos) == 1U);
    TEST_ASSERT(((startup_test_rtc.DR & RTC_DR_WDU_Msk) >> RTC_DR_WDU_Pos) == 1U);
    TEST_ASSERT(((startup_test_rtc.DR & RTC_DR_DU_Msk) >> RTC_DR_DU_Pos) == 1U);
    TEST_ASSERT(startup_test_rtc.TR == 0U); /* 00:00:00 */
}

static void test_reset_before_the_barrier_keeps_the_retained_clock_no_credit(void) {
    cold_boot(RCC_CSR_BORRSTF | RCC_CSR_PINRSTF);
    seed_state(ST_PHASE_PREPARED, 0, 0, 0);
    enter_startup();
    checkpoints = 0;
    armed = true;
    reset_after_checkpoint = 1; /* reset after the first, before the second, entry checkpoint */
    TEST_ASSERT(run_startup() == STOP_RESET);
    TEST_ASSERT(ons == 0);
    TEST_ASSERT(checkpoints == 1);
    TEST_ASSERT((RCC->CSR & RCC_CSR_BORRSTF) != 0U); /* never acknowledged before the barrier */

    /* Warm reboot: the reset did not lose power, so the calendar kept running and the
     * saved reference still looks continuous. The sticky power evidence must deny it. */
    clock_model_advance(2000);
    warm_boot(RCC_CSR_BORRSTF | RCC_CSR_PINRSTF);
    enter_startup();
    TEST_ASSERT(startup_clock_sample().valid);
    TEST_ASSERT(!startup_clock_retained());
    stop_at = model_epoch() + 120000;
    TEST_ASSERT(run_startup() == STOP_DEADLINE);
    TEST_ASSERT(ons == 0); /* unknown off-time and the interrupted wait are not credited */
    StSnapshot after = load();
    TEST_ASSERT(after.control.phase == ST_PHASE_INHIBIT);
    TEST_ASSERT(after.has_progress);
    TEST_ASSERT(after.progress.elapsed_ms <= 120000U);
    TEST_ASSERT(after.progress.reference_ms >= last_entry_epoch);
    /* The new boot's own barrier is what finally confirms and acknowledges the evidence. */
    TEST_ASSERT(checkpoints >= 2);
    TEST_ASSERT((RCC->CSR & RCC_CSR_BORRSTF) == 0U);
}

static void test_repeated_resets_before_the_barrier_stay_no_credit(void) {
    cold_boot(RCC_CSR_BORRSTF);
    seed_state(ST_PHASE_PREPARED, 0, 0, 0);
    enter_startup();
    for (unsigned round = 0; round < 3; ++round) {
        checkpoints = 0;
        armed = true;
        reset_after_checkpoint = 1;
        TEST_ASSERT(run_startup() == STOP_RESET);
        TEST_ASSERT(ons == 0);
        clock_model_advance(1500);
        warm_boot(RCC_CSR_BORRSTF); /* ordinary warm reset, power evidence still sticky */
        enter_startup();
        TEST_ASSERT(startup_clock_sample().valid);
        TEST_ASSERT(!startup_clock_retained());
        TEST_ASSERT((RCC->CSR & RCC_CSR_BORRSTF) != 0U);
    }
}

static void test_warm_reset_after_the_barrier_credits_verified_continuity(void) {
    /* Retained LSE domain with a preserved (non-seeded) calendar. */
    clock_model_lse_domain();
    clock_model_set_civil(2004, 2, 29, 1, 0, 0); /* leap day, retained */
    uint64_t entry = model_epoch();
    warm_boot(RCC_CSR_PINRSTF);
    /* Saved progress: 900000 ms elapsed, referenced 600000 ms ago on the same source. */
    seed_state(ST_PHASE_INHIBIT, 0, 900000, entry - 600000);
    enter_startup();
    TEST_ASSERT(startup_clock_sample().valid);
    TEST_ASSERT(startup_clock_retained()); /* ordinary reset, retained supported domain */
    input("START");
    input("ACK");

    TEST_ASSERT(run_startup() == 0); /* handed off to the scheduler */
    TEST_ASSERT(ons == 1);
    /* Only the remaining ~300 s was waited: saved progress plus verified off-time. */
    TEST_ASSERT(awake_waits > 0 && awake_waits <= 6);
    TEST_ASSERT(last_awake_wait_ms <= STARTUP_CHECKPOINT_MS);
    TEST_ASSERT((RCC->CSR & RCC_CSR_PINRSTF) == 0U); /* acknowledged after the barrier */
}

static void test_cross_day_wait_stays_coherent(void) {
    clock_model_lse_domain();
    clock_model_set_civil(2001, 1, 1, 23, 50, 0); /* wait runs across midnight */
    uint64_t entry = model_epoch();
    warm_boot(RCC_CSR_IWDGRSTF);
    seed_state(ST_PHASE_INHIBIT, 0, 0, entry);
    enter_startup();
    TEST_ASSERT(startup_clock_sample().valid);
    input("START");
    input("ACK");
    TEST_ASSERT(run_startup() == 0);
    TEST_ASSERT(ons == 1);
    TEST_ASSERT(on_epoch >= entry + 1800000ULL);
    TEST_ASSERT(on_epoch <= entry + 1800000ULL + 120000ULL);
    TEST_ASSERT(clock_model_ms >= 3600000ULL + 20ULL * 60000ULL); /* past midnight, 00:20 */
    TEST_ASSERT(awake_waits >= 30); /* the full inhibit really elapsed without RTC sleep */
    TEST_ASSERT(last_awake_wait_ms <= STARTUP_CHECKPOINT_MS);
}

/*
 * A reset between reservation and ON/OFF consumes the reservation. The retry wait it
 * starts must be referenced from a witnessed later sample: the plain pre-recovery
 * sample is quantity-floor-quantised earlier than the OFF command itself.
 */
static void test_reserved_recovery_persists_a_fresh_anchored_reference(void) {
    clock_model_lse_domain();
    clock_model_set_civil(2001, 1, 1, 0, 1, 40);
    warm_boot(RCC_CSR_PINRSTF);
    seed_state(ST_PHASE_RESERVED, 1, 0, 0);
    enter_startup();
    uint64_t entry = model_epoch();
    stop_at = entry + 20000;

    TEST_ASSERT(run_startup() == STOP_DEADLINE);
    TEST_ASSERT(ons == 0);
    TEST_ASSERT(fresh_latched); /* st_after_off persisted the retry reference */
    TEST_ASSERT(fresh_reference >= fresh_off_epoch + 2U * STARTUP_CLOCK_ANCHOR_STEP_MS);

    StSnapshot after = load();
    TEST_ASSERT(after.control.phase == ST_PHASE_RETRY_WAIT);
    TEST_ASSERT(after.control.reference_ms == fresh_reference);
    /* Conservative: the measured wait never runs ahead of the persisted reference. */
    if (after.has_progress) {
        TEST_ASSERT(after.progress.reference_ms >= fresh_reference);
        TEST_ASSERT(after.progress.elapsed_ms <= model_epoch() - after.progress.reference_ms);
    }
}

/*
 * A START that lands after the due reservation replaces the burn with OFF. That OFF's
 * fresh reference must also be a witnessed later sample, not the pre-OFF sample.
 */
static void test_late_start_after_reservation_persists_a_fresh_anchored_reference(void) {
    clock_model_lse_domain();
    clock_model_set_civil(2001, 1, 1, 0, 1, 40);
    uint64_t entry = model_epoch();
    warm_boot(RCC_CSR_PINRSTF);
    seed_state(ST_PHASE_RETRY_WAIT, 6, 172800000, entry - 172800000);
    enter_startup();
    input("PING");  /* consumed by the retry-wait listen loop */
    input("PING");  /* due reservation, no late START yet */
    input("START"); /* arrives after the reservation: OFF instead of ON */
    input("ACK");

    TEST_ASSERT(run_startup() == 0);
    TEST_ASSERT(ons == 0); /* the queued START replaced the burn */
    TEST_ASSERT(fresh_latched);
    TEST_ASSERT(fresh_reference >= fresh_off_epoch + 2U * STARTUP_CLOCK_ANCHOR_STEP_MS);
    StSnapshot after = load();
    TEST_ASSERT(after.control.phase == ST_PHASE_COMPLETE);
}

int main(void) {
    TEST_SUITE("real clock boundary inside the blocking Startup");
    RUN_TEST(test_cold_boot_seeds_the_fixed_interval_reference);
    RUN_TEST(test_reset_before_the_barrier_keeps_the_retained_clock_no_credit);
    RUN_TEST(test_repeated_resets_before_the_barrier_stay_no_credit);
    RUN_TEST(test_warm_reset_after_the_barrier_credits_verified_continuity);
    RUN_TEST(test_cross_day_wait_stays_coherent);
    RUN_TEST(test_reserved_recovery_persists_a_fresh_anchored_reference);
    RUN_TEST(test_late_start_after_reservation_persists_a_fresh_anchored_reference);
    TEST_RESULTS();
}
