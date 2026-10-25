#pragma once
/*
 * Host model of the RTC/RCC boundary the Startup interval clock calls.
 *
 * The model owns one civil instant (milliseconds since 2001-01-01T00:00:00) and
 * derives the modeled TR/DR/SSR words from it, advancing a fixed quantum per
 * register access so a polling reader can observe time passing. The decode uses
 * day-by-day civil arithmetic, deliberately independent of the closed-form
 * conversion in Src/startup/startup_clock.c.
 *
 * Limits: it is a register/time model. It is not STM32 bus timing, LSE
 * accuracy, shadow-register behaviour beyond the documented BYPSHAD direct
 * read, or electrical qualification. LSE startup is a modeled delay, not the
 * crystal's specification; calendar writes issued before the modeled source is
 * running are dropped, and RTC_CSR/PRER configuration writes are still applied.
 * The real rtc_config body is covered by
 * tests/startup/test_rtc_config_backup.py and the exact GNU13.3 ARM compile.
 */
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <RTC/rtc.h>
#include <globals.h>

#define CLOCK_MODEL_EPOCH_MS 978307200000ULL /* 2001-01-01T00:00:00, the seeded reference */
#define CLOCK_MODEL_SEED_YEAR 2001
#define CLOCK_MODEL_SUBSECOND_HZ 256U

StartupTestRTC startup_test_rtc;
StartupTestRCC startup_test_rcc;

uint64_t clock_model_ms;            /* modeled civil instant since 2001-01-01 */
uint64_t clock_model_monotonic_ms;  /* modeled getSysTime() */
uint32_t clock_model_ms_per_access = 1; /* bounded advance per register access */
bool clock_model_calendar_valid;    /* modeled calendar has decodable fields */
uint32_t clock_model_accesses;      /* counted register accesses */
bool clock_model_hold_registers;    /* keep poked register words instead of deriving them */
uint32_t clock_model_mutate_after_accesses; /* 0 = never: change a register mid-run */

/* LSE source model: LSERDY appears only after a modeled startup delay. */
uint32_t clock_model_lse_startup_ms;  /* delay after the LSE is (re)enabled */
uint64_t clock_model_lse_ready_at_ms; /* monotonic instant LSERDY may appear */
bool clock_model_lse_startup_pending; /* LSERDY still to appear after (re)enable */
bool clock_model_lse_absent;          /* modeled crystal that never starts */

/* Driver calls made by the module under test, to prove one non-forced request. */
uint32_t clock_model_rtc_config_calls;
uint32_t clock_model_rtc_config_forced;

/* The register a modeled mid-run change touches, at clock_model_mutate_after_accesses. */
#define CLOCK_MODEL_MUTATE_SOURCE 0U  /* RTCSEL switched to LSI */
#define CLOCK_MODEL_MUTATE_REFCKON 1U /* RTC_CR.REFCKON set */
#define CLOCK_MODEL_MUTATE_CALR 2U    /* RTC_CALR calibration written */
uint32_t clock_model_mutate_kind;

static inline bool clock_model_leap(uint32_t year) {
    return (year % 4U == 0U && (year % 100U != 0U || year % 400U == 0U));
}

static inline uint32_t clock_model_month_days(uint32_t year, uint32_t month) {
    static const uint8_t days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month < 1U || month > 12U) return 0U;
    return days[month - 1U] + (month == 2U && clock_model_leap(year) ? 1U : 0U);
}

/* Days from 2001-01-01, accumulated year by year so the arithmetic is explicit. */
static inline uint64_t clock_model_days_from_2001(uint32_t year, uint32_t month, uint32_t date) {
    uint64_t days = 0;
    for (uint32_t y = CLOCK_MODEL_SEED_YEAR; y < year; ++y) days += clock_model_leap(y) ? 366U : 365U;
    for (uint32_t m = 1; m < month; ++m) days += clock_model_month_days(year, m);
    return days + (date - 1U);
}

static inline uint32_t clock_model_bcd(uint32_t value) {
    return ((value / 10U) << 4) | (value % 10U);
}

static inline uint32_t clock_model_unbcd(uint32_t bcd) {
    return ((bcd >> 4) & 0xFU) * 10U + (bcd & 0xFU);
}

/* Derive the modeled calendar registers from the civil instant. */
static inline void clock_model_refresh(void) {
    if (!clock_model_calendar_valid || clock_model_hold_registers) return;
    uint64_t day_ms = clock_model_ms % 86400000ULL;
    uint64_t days = clock_model_ms / 86400000ULL;
    uint32_t seconds = (uint32_t)(day_ms / 1000ULL);
    uint32_t millis = (uint32_t)(day_ms % 1000ULL);
    uint32_t year = CLOCK_MODEL_SEED_YEAR;
    while (days >= (clock_model_leap(year) ? 366U : 365U)) {
        days -= clock_model_leap(year) ? 366U : 365U;
        ++year;
    }
    uint32_t month = 1;
    while (days >= clock_model_month_days(year, month)) {
        days -= clock_model_month_days(year, month);
        ++month;
    }
    uint32_t date = (uint32_t)days + 1U;
    uint32_t weekday = (uint32_t)((clock_model_ms / 86400000ULL) % 7ULL) + 1U; /* 2001-01-01 = Monday */
    startup_test_rtc.TR = (clock_model_bcd(seconds / 3600U) << 16)
        | (clock_model_bcd((seconds / 60U) % 60U) << 8) | clock_model_bcd(seconds % 60U);
    startup_test_rtc.DR = (clock_model_bcd((year - 2000U) / 10U) << 20)
        | (clock_model_bcd((year - 2000U) % 10U) << 16)
        | (weekday << 13) | (clock_model_bcd(month) << 8) | clock_model_bcd(date);
    /* SSR counts down from PREDIV_S to 0 inside each second. */
    startup_test_rtc.SSR = 255U - (millis * CLOCK_MODEL_SUBSECOND_HZ) / 1000U;
}

/* Recompute the civil instant from the calendar registers after a driver write. */
static inline void clock_model_sync(void) {
    uint32_t tr = startup_test_rtc.TR, dr = startup_test_rtc.DR;
    uint32_t year = 2000U + clock_model_unbcd((dr >> 20) & 0xFU) * 10U + clock_model_unbcd((dr >> 16) & 0xFU);
    uint32_t month = clock_model_unbcd((dr >> 8) & 0x1FU);
    uint32_t date = clock_model_unbcd(dr & 0x3FU);
    uint32_t hour = clock_model_unbcd((tr >> 16) & 0x3FU);
    uint32_t minute = clock_model_unbcd((tr >> 8) & 0x7FU);
    uint32_t second = clock_model_unbcd(tr & 0x7FU);
    if (year < CLOCK_MODEL_SEED_YEAR || month < 1U || month > 12U || date < 1U ||
        date > clock_model_month_days(year, month) || hour > 23U || minute > 59U || second > 59U) {
        clock_model_calendar_valid = false;
        return;
    }
    clock_model_calendar_valid = true;
    clock_model_ms = clock_model_days_from_2001(year, month, date) * 86400000ULL
        + ((uint64_t)hour * 3600ULL + (uint64_t)minute * 60ULL + second) * 1000ULL;
}

static inline void clock_model_reset(uint64_t start_ms) {
    memset(&startup_test_rtc, 0, sizeof(startup_test_rtc));
    memset(&startup_test_rcc, 0, sizeof(startup_test_rcc));
    clock_model_ms = start_ms;
    clock_model_monotonic_ms = 0;
    clock_model_ms_per_access = 1;
    clock_model_calendar_valid = false;
    clock_model_accesses = 0;
    clock_model_mutate_after_accesses = 0;
    clock_model_mutate_kind = CLOCK_MODEL_MUTATE_SOURCE;
    clock_model_hold_registers = false;
    clock_model_lse_startup_ms = 0;
    clock_model_lse_ready_at_ms = 0;
    clock_model_lse_startup_pending = false;
    clock_model_lse_absent = false;
    clock_model_rtc_config_calls = 0;
    clock_model_rtc_config_forced = 0;
}

static inline void clock_model_advance(uint64_t ms) {
    clock_model_monotonic_ms += ms;
    if (clock_model_calendar_valid) clock_model_ms += ms;
}

/* Set the modeled calendar directly, as a retained domain would present it. */
static inline void clock_model_set_civil(uint32_t year, uint32_t month, uint32_t date,
                                         uint32_t hour, uint32_t minute, uint32_t second) {
    clock_model_ms = clock_model_days_from_2001(year, month, date) * 86400000ULL
        + ((uint64_t)hour * 3600ULL + (uint64_t)minute * 60ULL + second) * 1000ULL;
    clock_model_calendar_valid = true;
    clock_model_refresh();
}

/* A retained, correctly configured LSE domain (warm boot without rtc_config work). */
static inline void clock_model_lse_domain(void) {
    startup_test_rcc.BDCR = RCC_BDCR_LSEON | RCC_BDCR_LSERDY | RCC_BDCR_RTCEN | RCC_BDCR_RTCSEL_0;
    startup_test_rtc.PRER = (127U << RTC_PRER_PREDIV_A_Pos) | (255U << RTC_PRER_PREDIV_S_Pos);
    startup_test_rtc.CR = RTC_CR_BYPSHAD;
    startup_test_rtc.ISR = RTC_ISR_INITS;
}

/*
 * A backup domain that lost power: no calendar and no initialized flags. The LSE
 * is requested by init_coreClocks() on every boot before the interval clock runs,
 * so LSEON is present while LSERDY only appears after the modeled startup delay.
 */
static inline void clock_model_lost_domain(void) {
    startup_test_rcc.BDCR = RCC_BDCR_LSEON;
    startup_test_rtc.ISR = 0;
    startup_test_rtc.PRER = 0;
    startup_test_rtc.CR = 0;
    startup_test_rtc.TR = 0;
    startup_test_rtc.DR = 0;
    startup_test_rtc.SSR = 0;
    startup_test_rtc.CALR = 0;
    clock_model_calendar_valid = false;
    clock_model_lse_ready_at_ms = clock_model_monotonic_ms + clock_model_lse_startup_ms;
    clock_model_lse_startup_pending = true;
}

/* --- Access hooks: every register access advances the bounded time model --- */
static inline StartupTestRTC *startup_test_rtc_registers(void) {
    clock_model_advance(clock_model_ms_per_access);
    clock_model_refresh();
    ++clock_model_accesses;
    if (clock_model_mutate_after_accesses != 0U &&
        clock_model_accesses == clock_model_mutate_after_accesses) {
        /* Modeled register change while the clock is being sampled. */
        switch (clock_model_mutate_kind) {
        case CLOCK_MODEL_MUTATE_REFCKON:
            startup_test_rtc.CR |= RTC_CR_REFCKON;
            break;
        case CLOCK_MODEL_MUTATE_CALR:
            startup_test_rtc.CALR = RTC_CALR_CALM_Msk;
            break;
        default:
            startup_test_rcc.BDCR =
                (startup_test_rcc.BDCR & ~RCC_BDCR_RTCSEL_Msk) | RCC_BDCR_RTCSEL_1;
            break;
        }
    }
    return &startup_test_rtc;
}

/*
 * The LSE reports ready once its modeled startup delay has elapsed. Only a pending
 * start raises the flag, so a test can still clear LSERDY to model an unusable
 * retained domain without the model immediately restoring it.
 */
static inline void clock_model_lse_tick(void) {
    if (!clock_model_lse_startup_pending || clock_model_lse_absent) return;
    if ((startup_test_rcc.BDCR & RCC_BDCR_LSEON) &&
        clock_model_monotonic_ms >= clock_model_lse_ready_at_ms) {
        startup_test_rcc.BDCR |= RCC_BDCR_LSERDY;
        clock_model_lse_startup_pending = false;
    }
}

static inline StartupTestRCC *startup_test_rcc_registers(void) {
    /* Wall-clock time keeps running during a poll that only reads RCC. */
    clock_model_advance(clock_model_ms_per_access);
    /* RCC_CSR.RMVF is write-one-to-clear: the clear lands on the next access. */
    if (startup_test_rcc.CSR & RCC_CSR_RMVF) {
        startup_test_rcc.CSR &= ~(RCC_CSR_RMVF | RCC_CSR_FWRSTF | RCC_CSR_OBLRSTF |
                                  RCC_CSR_PINRSTF | RCC_CSR_BORRSTF | RCC_CSR_SFTRSTF |
                                  RCC_CSR_IWDGRSTF | RCC_CSR_WWDGRSTF | RCC_CSR_LPWRRSTF);
    }
    clock_model_lse_tick();
    return &startup_test_rcc;
}

#undef RTC
#undef RCC
#define RTC startup_test_rtc_registers()
#define RCC startup_test_rcc_registers()

/* --- Existing RTC primitives, modeled at their documented contract --- */

/*
 * RTCCLK availability as the setters see it: the driver enters initialization mode
 * and waits DEFAULT_TIMEOUT_MS for INITF, so a calendar write issued before the
 * selected source is running is dropped. The LSI branch of the model has no startup
 * delay, so only the LSE selection can be unclocked.
 */
static inline bool clock_model_rtc_clocked(void) {
    uint32_t bdcr = startup_test_rcc.BDCR;
    if ((bdcr & RCC_BDCR_RTCSEL_Msk) == RCC_BDCR_RTCSEL_1) return true;
    return (bdcr & RCC_BDCR_LSERDY) != 0;
}

/* Src/system_config/RTC/rtc.c: non-forced configuration preserves an initialized RTC. */
void rtc_config(char clock_source, int forced_config) {
    ++clock_model_rtc_config_calls;
    if (forced_config) ++clock_model_rtc_config_forced;
    if ((startup_test_rtc.ISR & RTC_ISR_INITS) && !forced_config) return;
    /*
     * BDRST resets the backup domain: calendar, BKP registers and the BDCR bits the
     * driver then restores. It also restarts the LSE, so LSERDY only reappears after
     * the modeled startup delay - the driver's own 50 ms wait cannot cover that.
     */
    uint32_t backup = startup_test_rcc.BDCR & ~RCC_BDCR_RTCSEL_Msk;
    startup_test_rtc.ISR = 0;
    startup_test_rtc.TR = 0;
    startup_test_rtc.DR = 0;
    startup_test_rtc.SSR = 0;
    startup_test_rtc.CALR = 0;
    startup_test_rtc.BKP0R = startup_test_rtc.BKP1R = 0;
    clock_model_calendar_valid = false;
    startup_test_rcc.BDCR &= ~(RCC_BDCR_RTCSEL_Msk | RCC_BDCR_LSEON | RCC_BDCR_LSERDY |
                               RCC_BDCR_RTCEN);
    switch (clock_source) {
    case LSE:
        startup_test_rcc.BDCR |= RCC_BDCR_RTCSEL_0;
        startup_test_rtc.PRER = (127U << RTC_PRER_PREDIV_A_Pos) | (255U << RTC_PRER_PREDIV_S_Pos);
        break;
    case LSI:
        startup_test_rcc.BDCR |= RCC_BDCR_RTCSEL_1;
        startup_test_rtc.PRER = (127U << RTC_PRER_PREDIV_A_Pos) | (249U << RTC_PRER_PREDIV_S_Pos);
        break;
    default:
        break;
    }
    /* Restore from reset; this handles the LSE request made before rtc_config. */
    startup_test_rcc.BDCR |= backup;
    if (startup_test_rcc.BDCR & RCC_BDCR_LSEON) {
        clock_model_lse_ready_at_ms = clock_model_monotonic_ms + clock_model_lse_startup_ms;
        clock_model_lse_startup_pending = true;
    }
    startup_test_rcc.BDCR |= RCC_BDCR_RTCEN;
    startup_test_rtc.CR |= RTC_CR_BYPSHAD;
    startup_test_rtc.ISR |= RTC_ISR_INIT | RTC_ISR_INITF;
    startup_test_rtc.ISR &= ~(RTC_ISR_INIT | RTC_ISR_INITF);
    startup_test_rtc.ISR |= RTC_ISR_INITS;
}

/* Driver setters validate their inputs and silently keep the old value otherwise. */
void rtc_setCalendar(uint8_t year, uint8_t month, uint8_t date, uint8_t day) {
    if (year == 0 || year > 99 || month < January || month > 12 ||
        date == 0 || date > 31 || day < Monday || day > 7) return;
    if (!clock_model_rtc_clocked()) return;
    startup_test_rtc.DR = (clock_model_bcd(year / 10U) << 20) | (clock_model_bcd(year % 10U) << 16)
        | ((uint32_t)day << 13) | (clock_model_bcd(month) << 8) | clock_model_bcd(date);
    clock_model_sync();
}

void rtc_setTime(uint8_t hour, uint8_t minute, uint8_t second) {
    if (!clock_model_rtc_clocked()) return;
    startup_test_rtc.TR = (clock_model_bcd(hour / 10U) << RTC_TR_HT_Pos)
        | (clock_model_bcd(hour % 10U) << RTC_TR_HU_Pos)
        | (clock_model_bcd(minute / 10U) << RTC_TR_MNT_Pos)
        | (clock_model_bcd(minute % 10U) << RTC_TR_MNU_Pos)
        | (clock_model_bcd(second / 10U) << RTC_TR_ST_Pos)
        | (clock_model_bcd(second % 10U) << RTC_TR_SU_Pos);
    clock_model_sync();
}

void rtc_getTime(uint8_t *hour, uint8_t *minute, uint8_t *second) {
    uint32_t tr = startup_test_rtc.TR;
    *hour = (uint8_t)(clock_model_unbcd((tr & RTC_TR_HT_Msk) >> RTC_TR_HT_Pos) * 10U
                      + clock_model_unbcd((tr & RTC_TR_HU_Msk) >> RTC_TR_HU_Pos));
    *minute = (uint8_t)(clock_model_unbcd((tr & RTC_TR_MNT_Msk) >> RTC_TR_MNT_Pos) * 10U
                        + clock_model_unbcd((tr & RTC_TR_MNU_Msk) >> RTC_TR_MNU_Pos));
    *second = (uint8_t)(clock_model_unbcd((tr & RTC_TR_ST_Msk) >> RTC_TR_ST_Pos) * 10U
                        + clock_model_unbcd((tr & RTC_TR_SU_Msk) >> RTC_TR_SU_Pos));
}

void rtc_getCalendar(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *day) {
    uint32_t dr = startup_test_rtc.DR;
    *year = (uint8_t)(clock_model_unbcd((dr & RTC_DR_YT_Msk) >> RTC_DR_YT_Pos) * 10U
                      + clock_model_unbcd((dr & RTC_DR_YU_Msk) >> RTC_DR_YU_Pos));
    *month = (uint8_t)(clock_model_unbcd((dr & RTC_DR_MT_Msk) >> RTC_DR_MT_Pos) * 10U
                       + clock_model_unbcd((dr & RTC_DR_MU_Msk) >> RTC_DR_MU_Pos));
    *date = (uint8_t)(clock_model_unbcd((dr & RTC_DR_DT_Msk) >> RTC_DR_DT_Pos) * 10U
                      + clock_model_unbcd((dr & RTC_DR_DU_Msk) >> RTC_DR_DU_Pos));
    *day = (uint8_t)((dr & RTC_DR_WDU_Msk) >> RTC_DR_WDU_Pos);
}

/* Modeled monotonic time. TIM7 is not started here, so it never blocks. */
uint64_t getSysTime(void) { return clock_model_monotonic_ms; }
bool is_time_out(uint64_t start_time, uint64_t timeout_ms) {
    return (start_time + timeout_ms) <= clock_model_monotonic_ms;
}
/* Optional integration observer on the existing delay boundary, after time advances. */
#ifndef STARTUP_CLOCK_TEST_DELAY_HOOK
#define STARTUP_CLOCK_TEST_DELAY_HOOK(ms) ((void)(ms))
#endif
void delay_ms(uint64_t ms) { clock_model_advance(ms); STARTUP_CLOCK_TEST_DELAY_HOOK(ms); }
/* Core busy work (Src/system_config/core_config.c) is not modeled: it costs no host
 * time, so only the module's own poll cap bounds a wait against a frozen clock. */
void nop(long long nop_loops) { (void)nop_loops; }
