#ifndef STARTUP_PERSISTENCE_H
#define STARTUP_PERSISTENCE_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/*
 * Record Contract Revision 2.1 — Plain-C Domain Types & API
 *
 * Implements the bounded foundation substep (#46):
 *   - Golden wire format codec: Critical (96B x 2 @ 8, 104) and Progress (64B x 2 @ 200, 264)
 *   - Load & validation of all 7 legal wire phases and predecessor/successor pairs
 *   - Safe repair of unmirrored critical state
 *   - Initial inhibit lifecycle, timing measurement, and progress checkpoints
 *
 * Deployment-domain transitions use explicit reservation and one-use ON permits.
 */

typedef enum {
    ST_OK,
    ST_RECOVERABLE,
    ST_INHIBITED,
    ST_NOT_DUE,
    ST_INVALID,
    ST_IO,
    ST_CONFLICT,
    ST_EXHAUSTED,
    ST_CLOCK_INVALID,
    ST_REANCHOR_REQUIRED
} StResult;

typedef enum {
    ST_PHASE_PREPARED = 0,
    ST_PHASE_INHIBIT = 1,
    ST_PHASE_ELIGIBLE = 2,
    ST_PHASE_RESERVED = 3,
    ST_PHASE_RETRY_WAIT = 4,
    ST_PHASE_RETIRED = 5,
    ST_PHASE_COMPLETE = 6
} StPhase;

enum {
    ST_FLAG_INHIBIT_DONE = 1U
};

typedef struct {
    uint64_t arming;
    uint64_t sequence;
    uint64_t reservation;
    uint64_t wait;
    uint64_t duration_ms;
    uint64_t elapsed_ms;
    uint64_t reference_ms;
    uint32_t source;
    uint32_t flags;
    uint8_t phase;
    uint8_t fast_count;
    uint8_t authorization;
    uint8_t reference_valid;
} StControl;

typedef struct {
    uint64_t arming;
    uint64_t wait;
    uint64_t sequence;
    uint64_t elapsed_ms;
    uint64_t reference_ms;
    uint32_t source;
    uint8_t reference_valid;
} StProgress;

typedef struct {
    uint8_t raw[320];             /* all four records, exact bytes: FRAM 8..327 */
    StControl control;           /* advisory decoded view; APIs rederive from live/raw bytes */
    StProgress progress;         /* advisory decoded view, if present */
    bool has_progress;
    bool critical_mirrored;
    uint8_t critical_slot;       /* 0 for A (addr 8), 1 for B (addr 104) */
    uint8_t progress_slot;       /* 0 for A (addr 200), 1 for B (addr 264) */
} StSnapshot;

typedef struct {
    uint64_t epoch_ms;
    uint32_t source;
    uint32_t revision;
    bool valid;
} StClock;

typedef enum {
    ST_NO_OLD_TIME_CREDIT,
    ST_BOOT_CONTINUITY_PROVEN
} StEntry;

typedef struct {
    uint64_t token;
} StWaitCursor;

typedef struct {
    uint64_t token;
} StOnPermit;

/* Implemented Foundation Public API Subset */

/*
 * st_load:
 * Re-reads all 320 bytes (FRAM 8..327) and validates critical copies and progress slots.
 * Invalidates any volatile active wait cursor / permit (fresh recovery entry).
 * Returns:
 *   ST_OK: mirrored valid critical state loaded into *out.
 *   ST_RECOVERABLE: valid unmirrored critical survivor/successor loaded into *out (repair needed).
 *   ST_INVALID: corrupted, invalid invariants, or incompatible valid pair.
 *   ST_IO: hardware read failure.
 */
StResult st_load(StSnapshot *out);

/*
 * st_repair:
 * Verifies live FRAM matches expected->raw. If unmirrored, byte-identically mirrors the
 * selected survivor/successor into the other replica using verified write stages.
 * If already mirrored, verifies live FRAM and returns ST_OK with NO writes.
 */
StResult st_repair(const StSnapshot *expected, StSnapshot *out);

/*
 * st_start_inhibit:
 * Advances mirrored PREPARED (seq 0, auth 1) to INHIBIT (seq 1) with 1,800,000 ms duration
 * and valid baseline RTC sample. Re-reads and checks live FRAM matches expected->raw (CAS).
 * If authorization == 0, returns ST_INHIBITED without writes.
 */
StResult st_start_inhibit(const StSnapshot *expected, const StClock *clock, StSnapshot *out);

/*
 * st_wait_open:
 * Opens the single active domain wait cursor for INHIBIT or RETRY_WAIT.
 * Applies entry accounting (ST_BOOT_CONTINUITY_PROVEN credits elapsed only since valid
 * coherent saved reference; ST_NO_OLD_TIME_CREDIT anchors newly to current valid sample).
 * Every entry attempt revokes the previous cursor, including invalid clock/source entries.
 * Checks the complete live/raw snapshot; decoded caller fields never supply elapsed evidence.
 */
StResult st_wait_open(const StSnapshot *snapshot, const StClock *clock, StEntry entry,
                      StWaitCursor *cursor);

/*
 * st_wait_measure:
 * Measures elapsed time within the active cursor without persistent writes.
 * Rejects invalid clock with ST_CLOCK_INVALID; rejects backward time or source/revision
 * change with ST_REANCHOR_REQUIRED (invalidating cursor and discarding unsaved delta).
 * Failures report no more than durable elapsed; live-byte changes return ST_CONFLICT.
 */
StResult st_wait_measure(const StSnapshot *snapshot, StWaitCursor *cursor,
                         const StClock *clock, uint64_t *elapsed_ms);

/*
 * st_checkpoint:
 * Verifies live FRAM matches expected snapshot->raw (CAS). Writes alternating progress
 * slot with verified stages, updates progress sequence, and atomically rebinds BOTH
 * the cursor and returned snapshot. After no-credit entry the first checkpoint persists
 * saved elapsed with the entry anchor, even if called later; measured time is retained
 * separately. A subsequent checkpoint persists current elapsed/current reference.
 */
StResult st_checkpoint(const StSnapshot *snapshot, StWaitCursor *cursor,
                       const StClock *clock, StSnapshot *out);

/*
 * st_finish_inhibit:
 * Verifies elapsed >= 1,800,000 ms. If not, returns ST_NOT_DUE without critical writes.
 * If due, verifies any pending entry barrier and then the current-time progress
 * checkpoint before the mirrored INHIBIT -> ELIGIBLE successor; invalidates the cursor.
 */
StResult st_finish_inhibit(const StSnapshot *snapshot, StWaitCursor *cursor,
                           const StClock *clock, StSnapshot *out);

/* Deployment-domain transitions. No API performs hardware or time I/O. */
StResult st_reserve(const StSnapshot *snapshot, StWaitCursor *cursor,
                    const StClock *clock, StSnapshot *out, StOnPermit *permit);
StResult st_take_on_permit(const StSnapshot *snapshot, StOnPermit *permit);
StResult st_after_off(const StSnapshot *snapshot, const StClock *clock, StSnapshot *out);
StResult st_retire(const StSnapshot *snapshot, StSnapshot *out);
StResult st_complete(const StSnapshot *snapshot, StSnapshot *out);

#endif /* STARTUP_PERSISTENCE_H */
