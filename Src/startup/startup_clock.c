/*
 * startup_clock.c
 *
 * Startup-private interval clock built on the existing RTC primitives.
 *
 * The RTC calendar is treated as an interval reference, never as verified UTC:
 * a genuinely uninitialized calendar is seeded once with a fixed civil instant
 * and only elapsed differences are used as continuity evidence. Reset flags are
 * captured before initialization and acknowledged only after a durable wait
 * entry exists, so a reset between seeding and that barrier cannot credit
 * unknown off-time.
 */

#include "startup/startup_clock.h"

#include <RTC/rtc.h>
#include <globals.h>

/***************************** RESET EVIDENCE ********************************/

/* Resets that do not by themselves invalidate a retained RTC domain. */
#define STARTUP_CLOCK_ORDINARY_RESET_FLAGS \
    (RCC_CSR_PINRSTF | RCC_CSR_SFTRSTF | RCC_CSR_IWDGRSTF | RCC_CSR_WWDGRSTF | \
     RCC_CSR_OBLRSTF | RCC_CSR_FWRSTF)
/* Brown-out/power evidence: the RTC domain may have been disturbed. */
#define STARTUP_CLOCK_POWER_EVENT_FLAGS (RCC_CSR_BORRSTF)
/* A reset whose cause cannot justify continuity (illegal low-power entry/exit). */
#define STARTUP_CLOCK_UNEXPLAINED_RESET_FLAGS (RCC_CSR_LPWRRSTF)
#define STARTUP_CLOCK_ALL_RESET_FLAGS \
    (STARTUP_CLOCK_ORDINARY_RESET_FLAGS | STARTUP_CLOCK_POWER_EVENT_FLAGS | \
     STARTUP_CLOCK_UNEXPLAINED_RESET_FLAGS)

/***************************** CLOCK DOMAIN **********************************/

#define STARTUP_CLOCK_PREDIV_A 127U
#define STARTUP_CLOCK_PREDIV_S 255U

/* Bounded coherent-read attempts and their wall-clock timeout. */
#define STARTUP_CLOCK_SAMPLE_POLLS 32U
#define STARTUP_CLOCK_SAMPLE_TIMEOUT_MS 20U
/* Bounded anchor polling: the poll cap bounds a frozen TIM7, the timeout a live one. */
#define STARTUP_CLOCK_ANCHOR_POLLS 4096U
#define STARTUP_CLOCK_ANCHOR_TIMEOUT_MS 20U

/*
 * Bounded LSE readiness wait, used only before a fresh calendar is seeded.
 *
 * rtc_config() restarts the backup domain (BDRST) and only waits the driver's
 * DEFAULT_TIMEOUT_MS (50 ms) for LSERDY; the setters then enter initialization mode
 * and are dropped until the RTC is actually clocked. 2000 ms is an engineering
 * timeout budget, NOT a measured startup time or a physical guarantee:
 * a crystal that starts later still fails closed with no seed and no acknowledged
 * reset evidence.
 *
 * The wait is bounded twice. is_time_out() stops it against a live TIM7; the poll
 * cap stops it against a frozen one. Each poll burns
 * STARTUP_CLOCK_LSE_READY_POLL_NOP_LOOPS iterations of the existing nop() loop.
 * At 80 MHz, even one cycle per iteration makes the 4096-poll cap span 2048 ms;
 * loop overhead only increases that bound. This is not oscillator qualification.
 */
#define STARTUP_CLOCK_LSE_READY_TIMEOUT_MS 2000U
#define STARTUP_CLOCK_LSE_READY_POLLS 4096U
#define STARTUP_CLOCK_LSE_READY_POLL_NOP_LOOPS 40000

#define STARTUP_CLOCK_MS_PER_DAY 86400000ULL

typedef struct {
    uint32_t source;    /* RTCSEL selector, 1..3 as StClock expects */
    uint32_t revision;  /* sampled domain configuration identity */
    uint32_t prediv_s;  /* subsecond divider from the validated prescaler */
} StartupClockDomain;

typedef struct {
    uint32_t year;   /* 0..99, i.e. 2000..2099 */
    uint32_t month;  /* 1..12 */
    uint32_t date;   /* 1..31, validated against the month */
    uint32_t day;    /* 1..7, 1 = Monday */
    uint32_t hour;
    uint32_t minute;
    uint32_t second;
    uint32_t subsecond; /* RTC_SSR.SS, counts down from prediv_s */
    uint32_t prediv_s;  /* subsecond divider the sample was read with */
} StartupClockFields;

typedef struct {
    bool captured;
    bool calendar_initialized; /* RTC_ISR.INITS before initialization could change it */
    bool power_event;          /* unacknowledged brown-out/power evidence */
    bool unexplained_reset;    /* no reset evidence, or evidence we cannot justify */
} StartupClockBootEvidence;

static StartupClockBootEvidence s_boot;
static bool s_calendar_usable;

void startup_clock_capture_boot_evidence(void) {
    /* Never acknowledge here: the flags stay sticky until a durable wait entry exists. */
    uint32_t flags = RCC->CSR & STARTUP_CLOCK_ALL_RESET_FLAGS;
    s_boot.calendar_initialized = (RTC->ISR & RTC_ISR_INITS) != 0;
    s_boot.power_event = (flags & STARTUP_CLOCK_POWER_EVENT_FLAGS) != 0;
    s_boot.unexplained_reset = flags == 0U || (flags & STARTUP_CLOCK_UNEXPLAINED_RESET_FLAGS) != 0;
    s_boot.captured = true;
    /* A calendar whose state was never initialized is not evidence of anything yet. */
    s_calendar_usable = s_boot.calendar_initialized;
}

/***************************** CALENDAR MATH *********************************/

static bool startup_clock_is_leap(uint32_t year) {
    return (year % 4U == 0U && (year % 100U != 0U || year % 400U == 0U));
}

static uint32_t startup_clock_month_days(uint32_t year, uint32_t month) {
    static const uint8_t days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month < 1U || month > 12U) return 0U;
    return days[month - 1U] + (month == 2U && startup_clock_is_leap(year) ? 1U : 0U);
}

/* Days since 1970-01-01 for a proleptic Gregorian civil date (Hinnant's algorithm). */
static int64_t startup_clock_days_from_civil(int64_t year, uint32_t month, uint32_t date) {
    year -= month <= 2U;
    int64_t era = (year >= 0 ? year : year - 399) / 400;
    uint32_t year_of_era = (uint32_t)(year - era * 400);
    uint32_t shifted_month = (uint32_t)((int32_t)month + (month > 2U ? -3 : 9));
    uint32_t day_of_year = (153U * shifted_month + 2U) / 5U + date - 1U;
    uint32_t day_of_era = year_of_era * 365U + year_of_era / 4U - year_of_era / 100U + day_of_year;
    return era * 146097 + (int64_t)day_of_era - 719468;
}

/*
 * Epoch milliseconds in the standard numeric convention. The value is only an
 * interval reference: a seeded calendar is not the real UTC date and time.
 * RTC_SSR counts down inside the second, so the elapsed fraction is added to
 * the whole seconds rather than subtracted from them.
 */
static uint64_t startup_clock_epoch_ms(const StartupClockFields *fields) {
    int64_t days = startup_clock_days_from_civil(2000 + (int64_t)fields->year, fields->month, fields->date);
    uint64_t ms = (uint64_t)days * STARTUP_CLOCK_MS_PER_DAY;
    ms += ((uint64_t)fields->hour * 3600U + fields->minute * 60U + fields->second) * 1000U;
    ms += ((uint64_t)(fields->prediv_s - fields->subsecond) * 1000U) /
          ((uint64_t)fields->prediv_s + 1U);
    return ms;
}

/***************************** RAW REGISTER CHECKS ***************************/

static bool startup_clock_bcd2(uint32_t tens, uint32_t ones, uint32_t max, uint32_t *out) {
    if (tens > 9U || ones > 9U) return false;
    uint32_t value = tens * 10U + ones;
    if (value > max) return false;
    *out = value;
    return true;
}

/* Validate the raw BCD calendar words before any arithmetic trusts them. */
static bool startup_clock_decode_raw(uint32_t tr, uint32_t dr, uint32_t ss,
                                     uint32_t prediv_s, StartupClockFields *fields) {
    StartupClockFields decoded = {0};
    if (tr & RTC_TR_PM_Msk) return false; /* 12-hour/PM state is not a supported configuration */
    if (!startup_clock_bcd2((tr & RTC_TR_HT_Msk) >> RTC_TR_HT_Pos,
                            (tr & RTC_TR_HU_Msk) >> RTC_TR_HU_Pos, 23U, &decoded.hour)) return false;
    if (!startup_clock_bcd2((tr & RTC_TR_MNT_Msk) >> RTC_TR_MNT_Pos,
                            (tr & RTC_TR_MNU_Msk) >> RTC_TR_MNU_Pos, 59U, &decoded.minute)) return false;
    if (!startup_clock_bcd2((tr & RTC_TR_ST_Msk) >> RTC_TR_ST_Pos,
                            (tr & RTC_TR_SU_Msk) >> RTC_TR_SU_Pos, 59U, &decoded.second)) return false;
    if (!startup_clock_bcd2((dr & RTC_DR_YT_Msk) >> RTC_DR_YT_Pos,
                            (dr & RTC_DR_YU_Msk) >> RTC_DR_YU_Pos, 99U, &decoded.year)) return false;
    if (!startup_clock_bcd2((dr & RTC_DR_MT_Msk) >> RTC_DR_MT_Pos,
                            (dr & RTC_DR_MU_Msk) >> RTC_DR_MU_Pos, 12U, &decoded.month)) return false;
    if (decoded.month < 1U) return false;
    if (!startup_clock_bcd2((dr & RTC_DR_DT_Msk) >> RTC_DR_DT_Pos,
                            (dr & RTC_DR_DU_Msk) >> RTC_DR_DU_Pos, 31U, &decoded.date)) return false;
    if (decoded.date < 1U ||
        decoded.date > startup_clock_month_days(2000U + decoded.year, decoded.month)) return false;
    decoded.day = (dr & RTC_DR_WDU_Msk) >> RTC_DR_WDU_Pos;
    if (decoded.day < 1U || decoded.day > 7U) return false;
    if (ss > prediv_s) return false; /* SSR cannot exceed PREDIV_S */
    decoded.subsecond = ss;
    decoded.prediv_s = prediv_s;
    *fields = decoded;
    return true;
}

/***************************** DOMAIN VALIDATION *****************************/

/*
 * The LSE domain gate: LSE selected, enabled, ready and no clock-security failure.
 * Shared by the readiness wait and every sample so both agree on "usable".
 */
static bool startup_clock_lse_configured(uint32_t bdcr) {
    if ((bdcr & RCC_BDCR_RTCSEL_Msk) != RCC_BDCR_RTCSEL_0) return false; /* LSE only: no fallback */
    if ((bdcr & (RCC_BDCR_RTCEN | RCC_BDCR_LSEON | RCC_BDCR_LSERDY)) !=
        (RCC_BDCR_RTCEN | RCC_BDCR_LSEON | RCC_BDCR_LSERDY)) return false;
    return (bdcr & RCC_BDCR_LSECSSD) == 0;
}

/* Bounded wait for the LSE to come up after rtc_config itself restarted it. */
static bool startup_clock_wait_lse_ready(void) {
    uint64_t start = getSysTime();
    for (uint32_t poll = 0; poll < STARTUP_CLOCK_LSE_READY_POLLS; ++poll) {
        if (startup_clock_lse_configured(RCC->BDCR)) return true;
        if (is_time_out(start, STARTUP_CLOCK_LSE_READY_TIMEOUT_MS)) break;
        nop(STARTUP_CLOCK_LSE_READY_POLL_NOP_LOOPS);
    }
    return false;
}

/*
 * Validate the actual registers rather than the requested source: RTC enabled,
 * LSE selected and ready, no clock-security failure, calendar initialized and
 * not being reconfigured, expected prescalers, 24-hour direct-read format and no
 * reference-clock or calibration adjustment.
 */
static bool startup_clock_read_domain(StartupClockDomain *domain) {
    uint32_t bdcr = RCC->BDCR;
    uint32_t isr = RTC->ISR;
    uint32_t prer = RTC->PRER;
    uint32_t cr = RTC->CR;
    uint32_t calr = RTC->CALR;
    if (!startup_clock_lse_configured(bdcr)) return false;
    if (!(isr & RTC_ISR_INITS)) return false;
    if (isr & (RTC_ISR_INIT | RTC_ISR_INITF | RTC_ISR_RECALPF | RTC_ISR_SHPF)) return false;
    if (cr & RTC_CR_FMT) return false;       /* 12-hour format is not supported */
    if (cr & RTC_CR_REFCKON) return false;   /* reference-clock detection is an adjustment */
    if (calr != 0U) return false;            /* programmed calibration changes the rate */
    if (!(cr & RTC_CR_BYPSHAD)) return false; /* direct reads are required for coherence checks */
    if ((prer & RTC_PRER_PREDIV_A_Msk) != (STARTUP_CLOCK_PREDIV_A << RTC_PRER_PREDIV_A_Pos) ||
        (prer & RTC_PRER_PREDIV_S_Msk) != (STARTUP_CLOCK_PREDIV_S << RTC_PRER_PREDIV_S_Pos)) return false;
    domain->source = (bdcr & RCC_BDCR_RTCSEL_Msk) >> RCC_BDCR_RTCSEL_Pos;
    /*
     * Configuration signature: source plus validated prescalers. With the accepted
     * LSE/127/255 profile this has exactly one value. It identifies a configuration,
     * not a clock generation or a history: it cannot detect a calendar written by an
     * external debugger, and continuity is proven by the boot/reset/barrier evidence
     * together with exclusive Startup ownership of the calendar, not by this value.
     */
    domain->revision = ((domain->source & 0x3U) << 24) | (prer & 0x00FFFFFFU);
    domain->prediv_s = STARTUP_CLOCK_PREDIV_S;
    return true;
}

/*
 * One coherent bracket around the existing date/time getters. TR/DR must not
 * change and SSR must not wrap inside the bracket; the earlier subsecond value
 * is used, so the reported instant never runs ahead of the read.
 */
static bool startup_clock_read_fields(StartupClockDomain *domain, StartupClockFields *fields) {
    uint32_t tr_before, dr_before, ss_before, tr_after, dr_after, ss_after;
    uint8_t hour, minute, second, year, month, date, day;

    ss_before = RTC->SSR & RTC_SSR_SS_Msk;
    tr_before = RTC->TR;
    dr_before = RTC->DR;
    rtc_getTime(&hour, &minute, &second);
    rtc_getCalendar(&year, &month, &date, &day);
    ss_after = RTC->SSR & RTC_SSR_SS_Msk;
    tr_after = RTC->TR;
    dr_after = RTC->DR;

    if (tr_before != tr_after || dr_before != dr_after) return false; /* torn read: retry */
    if (ss_after > ss_before) return false; /* subsecond wrapped: retry */
    if (!startup_clock_decode_raw(tr_before, dr_before, ss_before, domain->prediv_s, fields)) return false;
    /* The existing getters must agree with the validated raw registers. */
    if (hour != fields->hour || minute != fields->minute || second != fields->second ||
        year != fields->year || month != fields->month || date != fields->date ||
        day != fields->day) return false;
    return true;
}

/* One validated sample attempt sequence, bounded by polls and wall-clock time. */
static StClock startup_clock_read(void) {
    StClock clock = {0};
    uint64_t start = getSysTime();
    for (uint32_t poll = 0; poll < STARTUP_CLOCK_SAMPLE_POLLS; ++poll) {
        StartupClockDomain before, after;
        StartupClockFields fields;
        if (!startup_clock_read_domain(&before)) return clock;
        if (startup_clock_read_fields(&before, &fields)) {
            if (!startup_clock_read_domain(&after)) return clock;
            if (before.revision == after.revision) {
                clock.epoch_ms = startup_clock_epoch_ms(&fields);
                clock.source = before.source;
                clock.revision = before.revision;
                clock.valid = true;
                return clock;
            }
        }
        if (is_time_out(start, STARTUP_CLOCK_SAMPLE_TIMEOUT_MS)) break;
    }
    return clock; /* a failed sample never looks valid */
}

/***************************** PUBLIC BEHAVIOUR ******************************/

void startup_clock_init(void) {
    if (!s_boot.captured) startup_clock_capture_boot_evidence();
    if (s_boot.calendar_initialized) {
        /* Original source, non-forced: an initialized RTC keeps its source and calendar. */
        rtc_config(LSE, false);
    } else if (s_boot.power_event) {
        /* A power event explains an uninitialized calendar; configure that domain once. */
        rtc_config(LSE, false);
        /* That call restarted the backup domain and only waited 50 ms for the LSE, so
           wait - bounded - for the source before the setters can take the calendar.
           No reseed is attempted after a late start: it fails closed instead. */
        if (!startup_clock_wait_lse_ready()) {
            s_calendar_usable = false;
            return;
        }
        /* Fixed interval reference 2001-01-01 (Monday) 00:00:00, not purported UTC. */
        rtc_setCalendar(1, January, 1, Monday);
        rtc_setTime(0, 0, 0);
    } else {
        /* Unknown uninitialized state without a power event: never erase or reseed it. */
        s_calendar_usable = false;
        return;
    }
    /* Bounded initial probe of the configured domain. */
    StClock probe = startup_clock_read();
    s_calendar_usable = probe.valid;
}

StClock startup_clock_sample(void) {
    StClock clock = {0};
    if (!s_boot.captured || !s_calendar_usable) return clock; /* fail closed */
    return startup_clock_read();
}

StClock startup_clock_anchor(void) {
    StClock invalid = {0};
    StClock first = startup_clock_sample();
    if (!first.valid) return invalid;
    uint64_t start = getSysTime();
    for (uint32_t poll = 0; poll < STARTUP_CLOCK_ANCHOR_POLLS; ++poll) {
        StClock later = startup_clock_sample();
        /* A domain or sample failure during anchoring invalidates the anchor. */
        if (!later.valid || later.source != first.source ||
            later.revision != first.revision) return invalid;
        if (later.epoch_ms >= first.epoch_ms + STARTUP_CLOCK_ANCHOR_STEP_MS) return later;
        if (is_time_out(start, STARTUP_CLOCK_ANCHOR_TIMEOUT_MS)) break;
    }
    return invalid;
}

bool startup_clock_retained(void) {
    if (!s_boot.captured || !s_calendar_usable) return false;
    /* INITS alone is not continuity: the durable saved reference is the proof. */
    return !s_boot.power_event && !s_boot.unexplained_reset;
}

bool startup_clock_confirm_wait(uint32_t source, uint32_t revision) {
    StClock now = startup_clock_sample();
    if (!now.valid || now.source != source || now.revision != revision) return false;
    /* Both verified entry checkpoints and this revalidated clock domain now exist,
       so the captured reset evidence may be acknowledged. Never done on boot. */
    RCC->CSR |= RCC_CSR_RMVF;
    s_boot.power_event = false;
    s_boot.unexplained_reset = false;
    return true;
}
