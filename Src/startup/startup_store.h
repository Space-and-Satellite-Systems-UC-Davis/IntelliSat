#ifndef STARTUP_STORE_H
#define STARTUP_STORE_H

#include <stdbool.h>
#include <stdint.h>

#include "FRAM/FRAM_layout.h"
#include "startup/startup_state.h"

/*
 * SSP1 Startup record — the complete persistence contract.
 *
 * One record is 20 bytes: ASCII "SSP1", u32 save sequence, resume state (an
 * MCUState value), attempts used, flags, remaining wait time and a CRC32, with
 * the commit marker 0xA5 last. Two slots (A at 8, B at 28) hold a complete
 * old-or-new record: a save only ever rewrites the inactive slot, so an
 * interrupted update leaves the previous record usable.
 *
 * Only what must survive a reset is stored. Elapsed time is not stored: the
 * remaining time is counted by the boot-local monotonic timer and any progress
 * that was not saved is deliberately lost on every reset.
 */

#define STARTUP_INHIBIT_MS 1800000u   /* 30 minute inhibit before the first attempt */
#define STARTUP_RETRY_MS 172800000u   /* 2 days while attempts_used < 7 */
#define STARTUP_WEEKLY_MS 604800000u  /* 7 days once every attempt was used */
#define STARTUP_ATTEMPT_LIMIT 7u      /* attempts saturate here: no catch-up bursts */

typedef struct {
    uint8_t resume_state;   /* MCUState value; 0, 1, 2, 3 and 6 are durable */
    uint8_t attempts_used;  /* 0..STARTUP_ATTEMPT_LIMIT, saturating */
    bool armed;             /* operator authorization for deployment */
    uint32_t remaining_ms;  /* not-yet-counted wait time; 0 when not waiting */
} StartupStoreState;

typedef enum {
    STARTUP_STORE_OK = 0,
    STARTUP_STORE_INVALID,   /* no usable record, ambiguous pair, or field out of range */
    STARTUP_STORE_IO,        /* FRAM read/write/verification failed: result unknown */
    STARTUP_STORE_EXHAUSTED  /* the save sequence cannot advance without wrapping */
} StartupStoreResult;

/* Wait owed after `attempts_used` attempts: two days, then weekly once saturated. */
uint32_t startup_store_retry_ms(uint8_t attempts_used);

/* Field validation of one record, without media access. */
bool startup_store_state_is_valid(const StartupStoreState *state);

/*
 * Reads both slots once and selects the usable record. One valid slot is used;
 * two valid slots resolve to the higher sequence, and equal sequences are only
 * accepted when the slots are byte-identical. There is no stored active-slot
 * pointer: the selection also decides which slot the next save overwrites.
 *
 * @returns STARTUP_STORE_OK with the selected record in *out, otherwise
 *          STARTUP_STORE_IO or STARTUP_STORE_INVALID
 */
StartupStoreResult startup_store_load(StartupStoreState *out);

/*
 * Publishes *state to the inactive slot under a new sequence: invalidate the
 * commit marker and verify it, write and verify the exact payload bytes, then
 * commit and verify the complete record. The active slot is left untouched.
 * A successful return means the new record is verified on the media; only then
 * may the caller act on it (in particular, command ON).
 *
 * @returns STARTUP_STORE_OK only when the new record is verified
 */
StartupStoreResult startup_store_save(const StartupStoreState *state);

#endif /* STARTUP_STORE_H */
