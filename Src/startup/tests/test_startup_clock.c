/*
 * Host tests for the Startup-private interval clock.
 *
 * The real Src/startup/startup_clock.c runs against the register/time model in
 * clock_fixture.h. Not an ARM build, LSE accuracy or electrical qualification.
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "framework.h"
#include "clock_fixture.h"
#include "../startup_clock.c"

#define SEED_EPOCH_MS 978307200000ULL /* independent literal: 2001-01-01T00:00:00 epoch ms */

/*
 * Independent literals for the documented LSE readiness budget and its poll cap.
 * The budget is an engineering timeout chosen above the typical LSE startup time,
 * not a physical guarantee; the cap only bounds a wait against a frozen TIM7.
 */
#define LSE_READY_BUDGET_MS 2000ULL
#define LSE_READY_POLL_CAP 4096U

static void retained_lse_domain(uint32_t reset_flags) {
    clock_model_reset(0);
    clock_model_lse_domain();
    clock_model_set_civil(2001, 1, 1, 0, 1, 40); /* 00:01:40 on the seeded civil date */
    RCC->CSR = reset_flags;
    startup_clock_capture_boot_evidence();
    startup_clock_init();
}

static void test_sample_reports_coherent_calendar_in_epoch_ms(void) {
    retained_lse_domain(RCC_CSR_PINRSTF);
    clock_model_set_civil(2001, 1, 1, 0, 1, 40); /* hold a known instant so the value is exact */
    clock_model_ms_per_access = 0;
    StClock clock = startup_clock_sample();
    TEST_ASSERT(clock.valid);
    TEST_ASSERT(clock.source == 1); /* RTCSEL_LSE */
    TEST_ASSERT(clock.revision == startup_clock_sample().revision);
    TEST_ASSERT(clock.epoch_ms == SEED_EPOCH_MS + 100000ULL);
    /* Floor quantisation of the subsecond counter only ever reports earlier instants. */
    clock_model_set_civil(2001, 1, 1, 0, 1, 40);
    clock_model_ms += 999; /* 00:01:40.999 reads as 40 s + 996 ms */
    StClock late = startup_clock_sample();
    TEST_ASSERT(late.valid);
    TEST_ASSERT(late.epoch_ms == SEED_EPOCH_MS + 100996ULL);
    TEST_ASSERT(SEED_EPOCH_MS + 100999ULL - late.epoch_ms <= 4ULL);
}

static void test_sample_tracks_cross_midnight_and_leap_day(void) {
    clock_model_reset(0);
    clock_model_lse_domain();
    clock_model_set_civil(2000, 2, 28, 23, 59, 59);
    RCC->CSR = RCC_CSR_PINRSTF;
    startup_clock_capture_boot_evidence();
    startup_clock_init();
    clock_model_set_civil(2000, 2, 28, 23, 59, 59); /* reset after the init probe advanced it */
    clock_model_ms_per_access = 0;
    StClock before = startup_clock_sample();
    TEST_ASSERT(before.valid);
    /* 2000-02-29T00:00:00 is exactly one day after 2000-02-28T23:59:59 plus one second. */
    clock_model_set_civil(2000, 2, 29, 0, 0, 0);
    StClock after = startup_clock_sample();
    TEST_ASSERT(after.valid);
    TEST_ASSERT(after.epoch_ms >= before.epoch_ms);
    TEST_ASSERT(after.epoch_ms - before.epoch_ms == 1000ULL);
    /* Month lengths are enforced: 2001 is not a leap year. */
    clock_model_hold_registers = true;
    startup_test_rtc.DR = (0x00U << 20) | (0x01U << 16) | (2U << 13) | (0x02U << 8) | 0x29U;
    TEST_ASSERT(!startup_clock_sample().valid);
    startup_test_rtc.DR = (0x00U << 20) | (0x01U << 16) | (4U << 13) | (0x02U << 8) | 0x29U;
    TEST_ASSERT(!startup_clock_sample().valid); /* 2001-02-29 does not exist */
}

static void test_sample_rejects_malformed_raw_bcd(void) {
    retained_lse_domain(RCC_CSR_PINRSTF);
    clock_model_ms_per_access = 0;
    clock_model_hold_registers = true; /* poke raw words the model would otherwise derive */
    uint32_t tr = startup_test_rtc.TR;
    uint32_t dr = startup_test_rtc.DR;

    startup_test_rtc.TR = tr | 0x0FU; /* second ones digit 0xF */
    TEST_ASSERT(!startup_clock_sample().valid);
    startup_test_rtc.TR = tr | (0x2U << 20) | (0x4U << 16); /* hour 24 */
    TEST_ASSERT(!startup_clock_sample().valid);
    startup_test_rtc.TR = tr | RTC_TR_PM_Msk;
    TEST_ASSERT(!startup_clock_sample().valid);
    startup_test_rtc.TR = tr;

    startup_test_rtc.DR = dr & ~(0xFUL << 8); /* month 0 */
    TEST_ASSERT(!startup_clock_sample().valid);
    startup_test_rtc.DR = dr & ~RTC_DR_WDU_Msk; /* weekday 0 */
    TEST_ASSERT(!startup_clock_sample().valid);
    startup_test_rtc.DR = dr | 0x09U; /* date 09 is fine, but 0x0A is not BCD */
    TEST_ASSERT(startup_clock_sample().valid);
    startup_test_rtc.DR = dr | 0x0AU;
    TEST_ASSERT(!startup_clock_sample().valid);
    startup_test_rtc.DR = dr;
    TEST_ASSERT(startup_clock_sample().valid);
    clock_model_hold_registers = false;
}

static void test_sample_rejects_unsupported_or_unstable_domain(void) {
    retained_lse_domain(RCC_CSR_PINRSTF);
    TEST_ASSERT(startup_clock_sample().valid);

    uint32_t bdcr = startup_test_rcc.BDCR, prer = startup_test_rtc.PRER, cr = startup_test_rtc.CR;
    uint32_t isr = startup_test_rtc.ISR;

    startup_test_rcc.BDCR = (bdcr & ~RCC_BDCR_RTCSEL_Msk) | RCC_BDCR_RTCSEL_1; /* retained LSI */
    TEST_ASSERT(!startup_clock_sample().valid);
    startup_test_rcc.BDCR = bdcr & ~RCC_BDCR_LSERDY;
    TEST_ASSERT(!startup_clock_sample().valid);
    startup_test_rcc.BDCR = bdcr & ~RCC_BDCR_RTCEN;
    TEST_ASSERT(!startup_clock_sample().valid);
    startup_test_rcc.BDCR = bdcr | RCC_BDCR_LSECSSD;
    TEST_ASSERT(!startup_clock_sample().valid);
    startup_test_rcc.BDCR = bdcr;

    startup_test_rtc.CR = cr | RTC_CR_FMT; /* 12-hour format */
    TEST_ASSERT(!startup_clock_sample().valid);
    startup_test_rtc.CR = cr & ~RTC_CR_BYPSHAD;
    TEST_ASSERT(!startup_clock_sample().valid);
    startup_test_rtc.CR = cr;

    startup_test_rtc.PRER = (127U << RTC_PRER_PREDIV_A_Pos) | (249U << RTC_PRER_PREDIV_S_Pos);
    TEST_ASSERT(!startup_clock_sample().valid);
    startup_test_rtc.PRER = prer;

    startup_test_rtc.ISR = isr | RTC_ISR_INITF; /* initialization in progress */
    TEST_ASSERT(!startup_clock_sample().valid);
    startup_test_rtc.ISR = isr & ~RTC_ISR_INITS;
    TEST_ASSERT(!startup_clock_sample().valid);
    startup_test_rtc.ISR = isr;
    TEST_ASSERT(startup_clock_sample().valid);
}

static void test_sample_exhausts_bounded_retries_on_a_torn_bracket(void) {
    retained_lse_domain(RCC_CSR_PINRSTF);
    uint32_t before = clock_model_accesses;
    clock_model_ms_per_access = 1000; /* every register access lands in a new second */
    StClock clock = startup_clock_sample();
    TEST_ASSERT(!clock.valid);
    TEST_ASSERT(clock_model_accesses - before <= 1000U); /* bounded retries, no hang */
}

static void test_init_preserves_a_retained_calendar(void) {
    clock_model_reset(0);
    clock_model_lse_domain();
    clock_model_set_civil(2019, 6, 1, 12, 0, 0);
    uint32_t dr = startup_test_rtc.DR, tr = startup_test_rtc.TR;
    RCC->CSR = RCC_CSR_BORRSTF | RCC_CSR_PINRSTF;
    startup_clock_capture_boot_evidence();
    startup_clock_init();
    TEST_ASSERT(startup_test_rtc.DR == dr && startup_test_rtc.TR == tr);
    StClock clock = startup_clock_sample();
    TEST_ASSERT(clock.valid);
    TEST_ASSERT(clock.epoch_ms >= 1559390400000ULL); /* independent literal: 2019-06-01T12:00:00Z */
    TEST_ASSERT(clock.epoch_ms < 1559390400000ULL + 100ULL);
}

static void test_init_seeds_only_a_power_lost_uninitialized_calendar(void) {
    clock_model_reset(0);
    clock_model_lost_domain();
    RCC->CSR = RCC_CSR_BORRSTF | RCC_CSR_PINRSTF;
    startup_clock_capture_boot_evidence();
    startup_clock_init();
    TEST_ASSERT(startup_test_rtc.TR == 0U);
    TEST_ASSERT(((startup_test_rtc.DR & RTC_DR_YT_Msk) >> RTC_DR_YT_Pos) == 0U);
    TEST_ASSERT(((startup_test_rtc.DR & RTC_DR_YU_Msk) >> RTC_DR_YU_Pos) == 1U);
    TEST_ASSERT(((startup_test_rtc.DR & RTC_DR_MT_Msk) >> RTC_DR_MT_Pos) == 0U);
    TEST_ASSERT(((startup_test_rtc.DR & RTC_DR_MU_Msk) >> RTC_DR_MU_Pos) == 1U);
    TEST_ASSERT(((startup_test_rtc.DR & RTC_DR_DT_Msk) >> RTC_DR_DT_Pos) == 0U);
    TEST_ASSERT(((startup_test_rtc.DR & RTC_DR_DU_Msk) >> RTC_DR_DU_Pos) == 1U);
    TEST_ASSERT(((startup_test_rtc.DR & RTC_DR_WDU_Msk) >> RTC_DR_WDU_Pos) == 1U); /* Monday */
    StClock clock = startup_clock_sample();
    TEST_ASSERT(clock.valid);
    TEST_ASSERT(clock.epoch_ms >= SEED_EPOCH_MS && clock.epoch_ms < SEED_EPOCH_MS + 50ULL);
    TEST_ASSERT(clock.source == 1);
}

static void test_init_fails_closed_on_an_unexplained_uninitialized_calendar(void) {
    clock_model_reset(0);
    clock_model_lost_domain();
    RCC->CSR = RCC_CSR_PINRSTF; /* ordinary reset, no power event */
    startup_clock_capture_boot_evidence();
    startup_clock_init();
    TEST_ASSERT(startup_test_rtc.TR == 0U && startup_test_rtc.DR == 0U); /* not erased, not reseeded */
    TEST_ASSERT(!startup_clock_sample().valid);
    TEST_ASSERT(!startup_clock_retained());

    clock_model_reset(0);
    clock_model_lost_domain();
    RCC->CSR = 0U; /* no reset evidence at all */
    startup_clock_capture_boot_evidence();
    startup_clock_init();
    TEST_ASSERT(!startup_clock_sample().valid);
    TEST_ASSERT(!startup_clock_retained());
}

static void test_init_rejects_a_retained_lsi_without_switching(void) {
    clock_model_reset(0);
    clock_model_lse_domain();
    startup_test_rcc.BDCR = (startup_test_rcc.BDCR & ~RCC_BDCR_RTCSEL_Msk) | RCC_BDCR_RTCSEL_1;
    startup_test_rtc.PRER = (127U << RTC_PRER_PREDIV_A_Pos) | (249U << RTC_PRER_PREDIV_S_Pos);
    clock_model_set_civil(2001, 1, 2, 0, 0, 0);
    RCC->CSR = RCC_CSR_PINRSTF;
    startup_clock_capture_boot_evidence();
    startup_clock_init();
    TEST_ASSERT((startup_test_rcc.BDCR & RCC_BDCR_RTCSEL_Msk) == RCC_BDCR_RTCSEL_1);
    TEST_ASSERT(!startup_clock_sample().valid);
}

static void test_sample_rejects_reference_clock_and_calibration_configuration(void) {
    retained_lse_domain(RCC_CSR_PINRSTF);
    TEST_ASSERT(startup_clock_sample().valid);
    uint32_t cr = startup_test_rtc.CR;

    /* Reference-clock detection turns the calendar into an adjusted reference. */
    startup_test_rtc.CR = cr | RTC_CR_REFCKON;
    TEST_ASSERT(!startup_clock_sample().valid);
    startup_test_rtc.CR = cr;
    TEST_ASSERT(startup_clock_sample().valid);

    /* Any programmed calibration/adjustment is an unsupported rate change. */
    startup_test_rtc.CALR = RTC_CALR_CALM_Msk;
    TEST_ASSERT(!startup_clock_sample().valid);
    startup_test_rtc.CALR = RTC_CALR_CALP_Msk;
    TEST_ASSERT(!startup_clock_sample().valid);
    startup_test_rtc.CALR = 0U;
    TEST_ASSERT(startup_clock_sample().valid);
}

static void test_sample_rejects_an_adjustment_written_within_the_bracket(void) {
    retained_lse_domain(RCC_CSR_PINRSTF);
    clock_model_accesses = 0;
    clock_model_mutate_kind = CLOCK_MODEL_MUTATE_CALR; /* written while sampling */
    clock_model_mutate_after_accesses = 5;
    TEST_ASSERT(!startup_clock_sample().valid);

    clock_model_accesses = 0;
    clock_model_mutate_kind = CLOCK_MODEL_MUTATE_REFCKON;
    clock_model_mutate_after_accesses = 6;
    TEST_ASSERT(!startup_clock_sample().valid);
}

static void test_rtc_settime_packs_a_nonzero_time_into_the_raw_word(void) {
    clock_model_reset(0);
    clock_model_lse_domain();
    /* 12:34:56 in BCD: HT=1 HU=2 MNT=3 MNU=4 ST=5 SU=6, i.e. 0x00123456. */
    rtc_setTime(12, 34, 56);
    TEST_ASSERT(startup_test_rtc.TR == 0x00123456U);
    uint8_t hour = 0, minute = 0, second = 0;
    rtc_getTime(&hour, &minute, &second);
    TEST_ASSERT(hour == 12U && minute == 34U && second == 56U);
}

static void test_init_waits_for_a_delayed_lse_before_seeding(void) {
    clock_model_reset(0);
    clock_model_lse_startup_ms = 400U; /* eight times the driver's own 50 ms wait */
    clock_model_lost_domain();
    RCC->CSR = RCC_CSR_BORRSTF | RCC_CSR_PINRSTF;
    startup_clock_capture_boot_evidence();
    startup_clock_init();

    /* One non-forced request: the LSE is never reset again while waiting. */
    TEST_ASSERT(clock_model_rtc_config_calls == 1U);
    TEST_ASSERT(clock_model_rtc_config_forced == 0U);
    TEST_ASSERT((startup_test_rcc.BDCR & RCC_BDCR_LSERDY) != 0U);

    /* The fixed interval reference was actually written, not dropped. */
    TEST_ASSERT(startup_test_rtc.TR == 0U);
    TEST_ASSERT(((startup_test_rtc.DR & RTC_DR_YU_Msk) >> RTC_DR_YU_Pos) == 1U);
    TEST_ASSERT(((startup_test_rtc.DR & RTC_DR_MU_Msk) >> RTC_DR_MU_Pos) == 1U);
    TEST_ASSERT(((startup_test_rtc.DR & RTC_DR_DU_Msk) >> RTC_DR_DU_Pos) == 1U);
    TEST_ASSERT(((startup_test_rtc.DR & RTC_DR_WDU_Msk) >> RTC_DR_WDU_Pos) == 1U);

    StClock clock = startup_clock_sample();
    TEST_ASSERT(clock.valid);
    TEST_ASSERT(clock.source == 1);
    TEST_ASSERT(clock.epoch_ms >= SEED_EPOCH_MS && clock.epoch_ms < SEED_EPOCH_MS + 200ULL);
    TEST_ASSERT(clock_model_monotonic_ms >= 400U);       /* it waited for the crystal */
    TEST_ASSERT(clock_model_monotonic_ms < 400U + 200U);  /* bounded, well inside the budget */
}

static void test_init_fails_closed_when_the_lse_never_starts(void) {
    clock_model_reset(0);
    clock_model_lse_absent = true;
    clock_model_lost_domain();
    RCC->CSR = RCC_CSR_BORRSTF | RCC_CSR_PINRSTF;
    startup_clock_capture_boot_evidence();
    startup_clock_init();

    TEST_ASSERT((startup_test_rcc.BDCR & RCC_BDCR_LSERDY) == 0U);
    TEST_ASSERT(startup_test_rtc.TR == 0U && startup_test_rtc.DR == 0U); /* no false calendar */
    TEST_ASSERT(!startup_clock_sample().valid);
    TEST_ASSERT(!startup_clock_retained());
    TEST_ASSERT((RCC->CSR & RCC_CSR_BORRSTF) != 0U); /* evidence never acknowledged */
    TEST_ASSERT(clock_model_monotonic_ms >= LSE_READY_BUDGET_MS);
    TEST_ASSERT(clock_model_monotonic_ms < LSE_READY_BUDGET_MS + 100ULL);
}

static void test_init_readiness_wait_is_bounded_with_a_frozen_timer(void) {
    clock_model_reset(0);
    clock_model_lse_absent = true;
    clock_model_lost_domain();
    clock_model_ms_per_access = 0; /* getSysTime() never advances: only the poll cap stops it */
    RCC->CSR = RCC_CSR_BORRSTF | RCC_CSR_PINRSTF;
    startup_clock_capture_boot_evidence();
    uint32_t before = clock_model_accesses;
    startup_clock_init();
    TEST_ASSERT(clock_model_accesses - before <= LSE_READY_POLL_CAP + 4U);
    TEST_ASSERT(!startup_clock_sample().valid);
    TEST_ASSERT(startup_test_rtc.TR == 0U && startup_test_rtc.DR == 0U);
    TEST_ASSERT(!startup_clock_retained());
}

static void test_anchor_returns_a_later_witnessed_sample(void) {
    retained_lse_domain(RCC_CSR_PINRSTF);
    StClock entry = startup_clock_sample();
    TEST_ASSERT(entry.valid);
    StClock anchor = startup_clock_anchor();
    TEST_ASSERT(anchor.valid);
    TEST_ASSERT(anchor.epoch_ms >= entry.epoch_ms + STARTUP_CLOCK_ANCHOR_STEP_MS);
    TEST_ASSERT(anchor.source == entry.source && anchor.revision == entry.revision);
}

static void test_anchor_fails_closed_when_the_calendar_cannot_advance(void) {
    retained_lse_domain(RCC_CSR_PINRSTF);
    /* Bus accesses continue, but the calendar words stop changing. */
    clock_model_hold_registers = true;
    TEST_ASSERT(!startup_clock_anchor().valid);
    TEST_ASSERT(startup_clock_sample().valid); /* the held value itself is a coherent sample */
}

static void test_anchor_fails_closed_when_the_counter_is_frozen(void) {
    retained_lse_domain(RCC_CSR_PINRSTF);
    clock_model_ms_per_access = 0; /* frozen calendar and frozen monotonic clock */
    uint64_t polls = clock_model_accesses;
    StClock anchor = startup_clock_anchor();
    TEST_ASSERT(!anchor.valid);
    TEST_ASSERT(startup_clock_retained());     /* a frozen read alone does not revoke evidence */
    TEST_ASSERT(clock_model_accesses > polls); /* it did poll, and the poll cap bounded it */
}

static void test_anchor_fails_closed_on_a_domain_change(void) {
    retained_lse_domain(RCC_CSR_PINRSTF);
    clock_model_accesses = 0;
    clock_model_mutate_after_accesses = 3; /* source changes mid-anchor */
    StClock anchor = startup_clock_anchor();
    TEST_ASSERT(!anchor.valid);
}

static void test_boot_evidence_stays_sticky_until_a_confirmed_wait(void) {
    retained_lse_domain(RCC_CSR_BORRSTF | RCC_CSR_PINRSTF);
    StClock clock = startup_clock_sample();
    TEST_ASSERT(clock.valid);
    TEST_ASSERT(!startup_clock_retained()); /* unacknowledged power event: no old-time credit */
    TEST_ASSERT(!startup_clock_confirm_wait(clock.source + 1, clock.revision)); /* wrong domain */
    TEST_ASSERT((RCC->CSR & RCC_CSR_BORRSTF) != 0U); /* still sticky */
    TEST_ASSERT(startup_clock_confirm_wait(clock.source, clock.revision));
    TEST_ASSERT((RCC->CSR & RCC_CSR_BORRSTF) == 0U); /* acknowledged after the barrier */
    TEST_ASSERT(startup_clock_retained());
}

static void test_unexplained_reset_denies_warm_credit(void) {
    retained_lse_domain(RCC_CSR_LPWRRSTF);
    TEST_ASSERT(startup_clock_sample().valid);
    TEST_ASSERT(!startup_clock_retained());

    retained_lse_domain(RCC_CSR_IWDGRSTF); /* ordinary watchdog reset */
    TEST_ASSERT(startup_clock_retained());
}

int main(void) {
    TEST_SUITE("startup interval clock");
    RUN_TEST(test_sample_reports_coherent_calendar_in_epoch_ms);
    RUN_TEST(test_sample_tracks_cross_midnight_and_leap_day);
    RUN_TEST(test_sample_rejects_malformed_raw_bcd);
    RUN_TEST(test_sample_rejects_unsupported_or_unstable_domain);
    RUN_TEST(test_sample_rejects_reference_clock_and_calibration_configuration);
    RUN_TEST(test_sample_rejects_an_adjustment_written_within_the_bracket);
    RUN_TEST(test_sample_exhausts_bounded_retries_on_a_torn_bracket);
    RUN_TEST(test_init_preserves_a_retained_calendar);
    RUN_TEST(test_init_seeds_only_a_power_lost_uninitialized_calendar);
    RUN_TEST(test_init_fails_closed_on_an_unexplained_uninitialized_calendar);
    RUN_TEST(test_init_rejects_a_retained_lsi_without_switching);
    RUN_TEST(test_rtc_settime_packs_a_nonzero_time_into_the_raw_word);
    RUN_TEST(test_init_waits_for_a_delayed_lse_before_seeding);
    RUN_TEST(test_init_fails_closed_when_the_lse_never_starts);
    RUN_TEST(test_init_readiness_wait_is_bounded_with_a_frozen_timer);
    RUN_TEST(test_anchor_returns_a_later_witnessed_sample);
    RUN_TEST(test_anchor_fails_closed_when_the_calendar_cannot_advance);
    RUN_TEST(test_anchor_fails_closed_when_the_counter_is_frozen);
    RUN_TEST(test_anchor_fails_closed_on_a_domain_change);
    RUN_TEST(test_boot_evidence_stays_sticky_until_a_confirmed_wait);
    RUN_TEST(test_unexplained_reset_denies_warm_credit);
    TEST_RESULTS();
}
