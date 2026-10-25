#ifndef STARTUP_CLOCK_H
#define STARTUP_CLOCK_H

#include "startup/startup_persistence.h"

/*
 * Startup-private RTC interval clock.
 *
 * The RTC calendar is an interval reference, not verified UTC: a genuinely
 * uninitialized calendar is seeded once with a fixed civil instant, and only
 * elapsed differences are used as evidence. The existing RTC public getters,
 * setters and wake-up timer are reused as they are; no RTC public API, epoch
 * framework or revision getter is added.
 *
 * Every function fails closed. An unusable or unexplained clock yields an
 * invalid StClock, no old-time credit and no acknowledged reset evidence.
 */

/*
 * Minimum advance required before a fresh interval reference is accepted.
 * With the 256 Hz LSE subsecond divider a floor-quantised sample lags its
 * nominal instant by less than this step, so a later sample that has advanced
 * by at least one step is witnessed to be at or after the entry instant.
 */
#define STARTUP_CLOCK_ANCHOR_STEP_MS 5u

/*
 * Capture the reset cause and the pre-initialization RTC configuration.
 * Must run before oscillator/RTC initialization can alter that evidence, and
 * never acknowledges the reset flags.
 */
void startup_clock_capture_boot_evidence(void);

/*
 * Request the original LSE source without forcing a reconfiguration, and seed
 * a fixed interval reference only when the captured calendar was genuinely
 * uninitialized and a power event explains it. A retained or unexplained
 * calendar is left as it is; an unusable configuration stays unusable.
 */
void startup_clock_init(void);

/* One validated, coherent sample of the current clock domain. */
StClock startup_clock_sample(void);

/*
 * Fresh interval entry: sample, then return an already witnessed later sample
 * that has advanced at least STARTUP_CLOCK_ANCHOR_STEP_MS. Bounded by a
 * wall-clock timeout and a poll cap; no valid sample on failure.
 */
StClock startup_clock_anchor(void);

/*
 * Whether this boot may credit a saved reference: a retained, supported clock
 * domain and a recognised ordinary reset with no unacknowledged power event.
 * INITS alone is not continuity evidence; the durable saved reference is.
 */
bool startup_clock_retained(void);

/*
 * Acknowledge the boot's reset evidence once the caller's durable entry
 * barrier exists, revalidating the sampled clock domain first.
 */
bool startup_clock_confirm_wait(uint32_t source, uint32_t revision);

#endif /* STARTUP_CLOCK_H */
