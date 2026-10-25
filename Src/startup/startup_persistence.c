#include "startup/startup_persistence.h"
#include "peripherals/FRAM/MB85RS256B.h"
#include "peripherals/FRAM/FRAM_layout.h"

#include <string.h>

#define STC3_COMMIT 0x33435453U /* "STC3" little-endian */
#define STP3_COMMIT 0x33505453U /* "STP3" little-endian */

#define CRITICAL_VERSION 3U
#define CRITICAL_LENGTH 96U

#define PROGRESS_VERSION 3U
#define PROGRESS_LENGTH 64U

#define INHIBIT_DURATION_MS 1800000ULL
#define RETRY_WAIT_SHORT_MS 172800000ULL
#define RETRY_WAIT_LONG_MS  604800000ULL

/* Module-private active domain wait cursor */
typedef struct {
    uint64_t token;
    bool active;
    uint8_t raw_snapshot[FRAM_STARTUP_SNAPSHOT_SIZE];
    uint64_t arming;
    uint64_t wait_id;
    uint64_t duration_ms;
    uint64_t baseline_elapsed_ms;
    uint64_t durable_elapsed_ms;
    uint64_t durable_reference_ms;
    uint32_t durable_source;
    uint8_t durable_ref_valid;
    uint64_t last_epoch_ms;
    uint32_t last_source;
    uint32_t last_revision;
    uint64_t measured_elapsed_ms;
    bool entry_checkpoint_pending;
    uint64_t entry_elapsed_ms;
    StClock entry_clock;
} PrivateWaitCursor;

static PrivateWaitCursor s_active_cursor = {0};
static uint64_t s_cursor_token_counter = 1ULL;
static uint64_t s_permit_token_counter = 1ULL;
static uint64_t s_active_permit_token;
static uint8_t s_active_permit_available;
static uint8_t s_active_permit_raw[FRAM_STARTUP_SNAPSHOT_SIZE];

static void invalidate_permit(void) {
    s_active_permit_token = 0;
    s_active_permit_available = 0;
    memset(s_active_permit_raw, 0, sizeof(s_active_permit_raw));
}

/* Endian helpers */
static inline uint16_t read_u16_le(const uint8_t *p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static inline void write_u16_le(uint8_t *p, uint16_t v) {
    p[0] = (uint8_t)(v & 0xFFU);
    p[1] = (uint8_t)((v >> 8) & 0xFFU);
}

static inline uint32_t read_u32_le(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static inline void write_u32_le(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v & 0xFFU);
    p[1] = (uint8_t)((v >> 8) & 0xFFU);
    p[2] = (uint8_t)((v >> 16) & 0xFFU);
    p[3] = (uint8_t)((v >> 24) & 0xFFU);
}

static inline uint64_t read_u64_le(const uint8_t *p) {
    uint64_t v = 0;
    for (int i = 0; i < 8; i++) {
        v |= ((uint64_t)p[i]) << (8 * i);
    }
    return v;
}

static inline void write_u64_le(uint8_t *p, uint64_t v) {
    for (int i = 0; i < 8; i++) {
        p[i] = (uint8_t)((v >> (8 * i)) & 0xFFU);
    }
}

/* CRC32: reflected polynomial 0xEDB88320, init 0xFFFFFFFF, final XOR 0xFFFFFFFF */
static uint32_t crc32_reflected(const uint8_t *data, size_t length) {
    uint32_t crc = 0xFFFFFFFFU;
    for (size_t i = 0; i < length; i++) {
        crc ^= (uint32_t)data[i];
        for (int b = 0; b < 8; b++) {
            if (crc & 1U) {
                crc = (crc >> 1) ^ 0xEDB88320U;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc ^ 0xFFFFFFFFU;
}

/* Codec: Decode Critical record */
static void decode_critical_record(const uint8_t *raw, StControl *c) {
    c->arming = read_u64_le(&raw[8]);
    c->sequence = read_u64_le(&raw[16]);
    c->reservation = read_u64_le(&raw[24]);
    c->wait = read_u64_le(&raw[32]);
    c->duration_ms = read_u64_le(&raw[40]);
    c->elapsed_ms = read_u64_le(&raw[48]);
    c->reference_ms = read_u64_le(&raw[56]);
    c->source = read_u32_le(&raw[64]);
    c->flags = read_u32_le(&raw[68]);
    c->phase = raw[72];
    c->fast_count = raw[73];
    c->authorization = raw[74];
    c->reference_valid = raw[75];
}

/* Codec: Encode Critical record */
static void encode_critical_record(const StControl *c, uint8_t *raw) {
    memset(raw, 0, FRAM_CRITICAL_RECORD_SIZE);
    write_u32_le(&raw[0], STC3_COMMIT);
    write_u16_le(&raw[4], CRITICAL_VERSION);
    write_u16_le(&raw[6], CRITICAL_LENGTH);
    write_u64_le(&raw[8], c->arming);
    write_u64_le(&raw[16], c->sequence);
    write_u64_le(&raw[24], c->reservation);
    write_u64_le(&raw[32], c->wait);
    write_u64_le(&raw[40], c->duration_ms);
    write_u64_le(&raw[48], c->elapsed_ms);
    write_u64_le(&raw[56], c->reference_ms);
    write_u32_le(&raw[64], c->source);
    write_u32_le(&raw[68], c->flags);
    raw[72] = c->phase;
    raw[73] = c->fast_count;
    raw[74] = c->authorization;
    raw[75] = c->reference_valid;
    /* bytes 76..91 remain zero */
    uint32_t crc = crc32_reflected(&raw[4], 88);
    write_u32_le(&raw[92], crc);
}

/* Codec: Decode Progress record */
static void decode_progress_record(const uint8_t *raw, StProgress *p) {
    p->arming = read_u64_le(&raw[8]);
    p->wait = read_u64_le(&raw[16]);
    p->sequence = read_u64_le(&raw[24]);
    p->elapsed_ms = read_u64_le(&raw[32]);
    p->reference_ms = read_u64_le(&raw[40]);
    p->source = read_u32_le(&raw[48]);
    p->reference_valid = raw[52];
}

/* Codec: Encode Progress record */
static void encode_progress_record(const StProgress *p, uint8_t *raw) {
    memset(raw, 0, FRAM_PROGRESS_RECORD_SIZE);
    write_u32_le(&raw[0], STP3_COMMIT);
    write_u16_le(&raw[4], PROGRESS_VERSION);
    write_u16_le(&raw[6], PROGRESS_LENGTH);
    write_u64_le(&raw[8], p->arming);
    write_u64_le(&raw[16], p->wait);
    write_u64_le(&raw[24], p->sequence);
    write_u64_le(&raw[32], p->elapsed_ms);
    write_u64_le(&raw[40], p->reference_ms);
    write_u32_le(&raw[48], p->source);
    raw[52] = p->reference_valid;
    /* bytes 53..59 remain zero */
    uint32_t crc = crc32_reflected(&raw[4], 56);
    write_u32_le(&raw[60], crc);
}

static bool is_clear_wait(const StControl *c) {
    return (c->wait == 0 && c->duration_ms == 0 && c->elapsed_ms == 0 &&
            c->reference_ms == 0 && c->source == 0 && c->reference_valid == 0);
}

/* Invariant validation: Critical record */
static bool validate_critical_record(const uint8_t *raw, StControl *c) {
    if (read_u32_le(&raw[0]) != STC3_COMMIT) return false;
    if (read_u16_le(&raw[4]) != CRITICAL_VERSION) return false;
    if (read_u16_le(&raw[6]) != CRITICAL_LENGTH) return false;

    /* Reserved bytes 76..91 must be zero */
    for (int i = 76; i <= 91; i++) {
        if (raw[i] != 0) return false;
    }

    uint32_t stored_crc = read_u32_le(&raw[92]);
    uint32_t calc_crc = crc32_reflected(&raw[4], 88);
    if (stored_crc != calc_crc) return false;

    decode_critical_record(raw, c);

    if (c->arming == 0) return false;
    if (c->authorization > 1) return false;
    if (c->reference_valid > 1) return false;
    if ((c->flags & ~1U) != 0) return false;

    if (c->reference_valid == 1) {
        if (c->source < 1 || c->source > 3) return false;
    } else {
        if (c->reference_ms != 0 || c->source != 0) return false;
    }

    switch (c->phase) {
    case ST_PHASE_PREPARED:
        if (c->sequence != 0 || c->fast_count != 0 || c->reservation != 0 || c->flags != 0) return false;
        if (!is_clear_wait(c)) return false;
        break;

    case ST_PHASE_INHIBIT:
        if (c->authorization != 1) return false;
        if (c->sequence != 1 || c->wait != 1 || c->fast_count != 0 || c->reservation != 0 || c->flags != 0) return false;
        if (c->duration_ms != INHIBIT_DURATION_MS || c->elapsed_ms != 0) return false;
        if (c->reference_valid != 1 || c->source < 1 || c->source > 3) return false;
        break;

    case ST_PHASE_ELIGIBLE:
        if (c->authorization != 1) return false;
        if (c->sequence != 2 || c->fast_count != 0 || c->reservation != 0 || c->flags != 1) return false;
        if (!is_clear_wait(c)) return false;
        break;

    case ST_PHASE_RESERVED: {
        if (c->authorization != 1) return false;
        if ((c->sequence % 2 == 0) || c->sequence < 3) return false;
        if (c->reservation != c->sequence) return false;
        uint64_t expected_count = (c->sequence - 1) / 2;
        if (expected_count > 7) expected_count = 7;
        if (c->fast_count != (uint8_t)expected_count) return false;
        if (c->flags != 1) return false;
        if (!is_clear_wait(c)) return false;
        break;
    }

    case ST_PHASE_RETRY_WAIT: {
        if (c->authorization != 1) return false;
        if ((c->sequence % 2 != 0) || c->sequence < 4) return false;
        if (c->reservation != c->sequence - 1) return false;
        if (c->wait != c->sequence) return false;
        uint64_t expected_count = c->reservation / 2;
        if (expected_count > 7) expected_count = 7;
        if (c->fast_count != (uint8_t)expected_count) return false;
        if (c->flags != 1 || c->elapsed_ms != 0) return false;
        uint64_t expected_dur = (c->fast_count < 7) ? RETRY_WAIT_SHORT_MS : RETRY_WAIT_LONG_MS;
        if (c->duration_ms != expected_dur) return false;
        if (c->reference_valid != 1 || c->source < 1 || c->source > 3) return false;
        break;
    }

    case ST_PHASE_RETIRED: {
        if (c->authorization != 1 || c->flags != 1) return false;
        if (!is_clear_wait(c)) return false;
        if (c->reservation == 0 && c->fast_count == 0) {
            if (c->sequence != 3) return false;
        } else {
            if ((c->reservation % 2 == 0) || c->reservation < 3) return false;
            if (c->reservation > UINT64_MAX - 2) return false;
            if (c->sequence != c->reservation + 2) return false;
            uint64_t expected_count = c->reservation / 2;
            if (expected_count > 7) expected_count = 7;
            if (c->fast_count != (uint8_t)expected_count) return false;
        }
        break;
    }

    case ST_PHASE_COMPLETE: {
        if (c->authorization != 1 || c->flags != 1) return false;
        if (!is_clear_wait(c)) return false;
        if (c->reservation == 0 && c->fast_count == 0) {
            if (c->sequence != 4) return false;
        } else {
            if ((c->reservation % 2 == 0) || c->reservation < 3) return false;
            if (c->reservation > UINT64_MAX - 3) return false;
            if (c->sequence != c->reservation + 3) return false;
            uint64_t expected_count = c->reservation / 2;
            if (expected_count > 7) expected_count = 7;
            if (c->fast_count != (uint8_t)expected_count) return false;
        }
        break;
    }

    default:
        return false;
    }

    return true;
}

/* Exact legal successor transition predicate */
static bool is_legal_successor_transition(const StControl *prev, const StControl *next) {
    if (prev->sequence == UINT64_MAX) return false;
    if (next->sequence != prev->sequence + 1) return false;
    if (next->arming != prev->arming) return false;
    if (next->authorization != prev->authorization) return false;

    if (prev->phase == ST_PHASE_PREPARED && next->phase == ST_PHASE_INHIBIT) {
        if (prev->authorization != 1) return false;
        return (next->sequence == 1 && next->wait == 1 &&
                next->duration_ms == INHIBIT_DURATION_MS && next->elapsed_ms == 0 &&
                next->reference_valid == 1 && next->source >= 1 && next->source <= 3);
    }

    if (prev->phase == ST_PHASE_INHIBIT && next->phase == ST_PHASE_ELIGIBLE) {
        return (next->sequence == 2 && next->flags == 1 && is_clear_wait(next));
    }

    if ((prev->phase == ST_PHASE_ELIGIBLE || prev->phase == ST_PHASE_RETRY_WAIT) &&
        next->phase == ST_PHASE_RESERVED) {
        uint8_t expected_count = (uint8_t)(prev->fast_count + 1 < 7 ? prev->fast_count + 1 : 7);
        return (next->reservation == next->sequence && next->fast_count == expected_count &&
                next->flags == 1 && is_clear_wait(next));
    }

    if (prev->phase == ST_PHASE_RESERVED && next->phase == ST_PHASE_RETRY_WAIT) {
        return (next->reservation == prev->reservation && next->fast_count == prev->fast_count &&
                next->wait == next->sequence && next->elapsed_ms == 0 &&
                next->reference_valid == 1 && next->source >= 1 && next->source <= 3 &&
                next->flags == 1);
    }

    if ((prev->phase == ST_PHASE_ELIGIBLE || prev->phase == ST_PHASE_RETRY_WAIT) &&
        next->phase == ST_PHASE_RETIRED) {
        return (next->reservation == prev->reservation && next->fast_count == prev->fast_count &&
                next->flags == 1 && is_clear_wait(next));
    }

    if (prev->phase == ST_PHASE_RETIRED && next->phase == ST_PHASE_COMPLETE) {
        return (next->reservation == prev->reservation && next->fast_count == prev->fast_count &&
                next->flags == 1 && is_clear_wait(next));
    }

    return false;
}

/* Invariant validation: Progress record */
static bool validate_progress_record(const uint8_t *raw, StProgress *p, const StControl *critical) {
    if (read_u32_le(&raw[0]) != STP3_COMMIT) return false;
    if (read_u16_le(&raw[4]) != PROGRESS_VERSION) return false;
    if (read_u16_le(&raw[6]) != PROGRESS_LENGTH) return false;

    for (int i = 53; i <= 59; i++) {
        if (raw[i] != 0) return false;
    }

    uint32_t stored_crc = read_u32_le(&raw[60]);
    uint32_t calc_crc = crc32_reflected(&raw[4], 56);
    if (stored_crc != calc_crc) return false;

    decode_progress_record(raw, p);

    if (critical->phase != ST_PHASE_INHIBIT && critical->phase != ST_PHASE_RETRY_WAIT) {
        return false;
    }

    if (p->arming != critical->arming) return false;
    if (p->wait != critical->wait) return false;
    if (p->sequence < 1) return false;
    if (p->elapsed_ms < critical->elapsed_ms || p->elapsed_ms > critical->duration_ms) return false;

    if (p->reference_valid > 1) return false;
    if (p->reference_valid == 1) {
        if (p->source < 1 || p->source > 3) return false;
    } else {
        if (p->reference_ms != 0 || p->source != 0) return false;
    }

    return true;
}

/* Hardware write stages with verified readback */
static bool write_verified_stage(uint16_t addr, const uint8_t *data, uint16_t size) {
    if (size > FRAM_CRITICAL_RECORD_SIZE) return false;
    if (!FRAM_writeEnable()) {
        (void)FRAM_writeDisable(); /* attempted cleanup */
        return false;
    }
    uint8_t temp[FRAM_CRITICAL_RECORD_SIZE];
    memcpy(temp, data, size);
    bool write_ok = FRAM_writeData(addr, temp, size);
    bool wrdi_ok = FRAM_writeDisable();
    if (!write_ok || !wrdi_ok) return false;

    uint8_t readback[FRAM_CRITICAL_RECORD_SIZE];
    if (!FRAM_readData(addr, readback, size)) return false;
    if (memcmp(readback, data, size) != 0) return false;
    return true;
}

static bool write_critical_record(uint16_t addr, const uint8_t *rec) {
    /* Stage 1: Invalidate destination commit */
    uint8_t zero_commit[4] = {0, 0, 0, 0};
    if (!write_verified_stage(addr, zero_commit, 4)) return false;

    /* Stage 2: Write payload (offsets 4..95) */
    if (!write_verified_stage((uint16_t)(addr + 4), &rec[4], 92)) return false;

    /* Stage 3: Write commit */
    if (!write_verified_stage(addr, &rec[0], 4)) return false;

    /* Stage 4: Full record readback and verification */
    uint8_t full_read[FRAM_CRITICAL_RECORD_SIZE];
    if (!FRAM_readData(addr, full_read, FRAM_CRITICAL_RECORD_SIZE)) return false;
    if (memcmp(full_read, rec, FRAM_CRITICAL_RECORD_SIZE) != 0) return false;
    return true;
}

static bool write_progress_record(uint16_t addr, const uint8_t *rec) {
    /* Stage 1: Invalidate destination commit */
    uint8_t zero_commit[4] = {0, 0, 0, 0};
    if (!write_verified_stage(addr, zero_commit, 4)) return false;

    /* Stage 2: Write payload (offsets 4..63) */
    if (!write_verified_stage((uint16_t)(addr + 4), &rec[4], 60)) return false;

    /* Stage 3: Write commit */
    if (!write_verified_stage(addr, &rec[0], 4)) return false;

    /* Stage 4: Full record readback and verification */
    uint8_t full_read[FRAM_PROGRESS_RECORD_SIZE];
    if (!FRAM_readData(addr, full_read, FRAM_PROGRESS_RECORD_SIZE)) return false;
    if (memcmp(full_read, rec, FRAM_PROGRESS_RECORD_SIZE) != 0) return false;
    return true;
}

/* Internal snapshot decoder from raw 320 bytes */
static StResult decode_snapshot_raw(const uint8_t *raw, StSnapshot *out) {
    memcpy(out->raw, raw, FRAM_STARTUP_SNAPSHOT_SIZE);

    const uint8_t *raw_crit_a = &raw[0];
    const uint8_t *raw_crit_b = &raw[FRAM_CRITICAL_RECORD_SIZE];

    StControl crit_a, crit_b;
    bool val_a = validate_critical_record(raw_crit_a, &crit_a);
    bool val_b = validate_critical_record(raw_crit_b, &crit_b);

    if (val_a && val_b) {
        if (crit_a.sequence == crit_b.sequence) {
            if (memcmp(raw_crit_a, raw_crit_b, FRAM_CRITICAL_RECORD_SIZE) != 0) {
                return ST_INVALID;
            }
            out->control = crit_a;
            out->critical_mirrored = true;
            out->critical_slot = 0;
        } else if (crit_a.sequence > crit_b.sequence && crit_a.sequence - crit_b.sequence == 1) {
            if (!is_legal_successor_transition(&crit_b, &crit_a)) return ST_INVALID;
            out->control = crit_a;
            out->critical_mirrored = false;
            out->critical_slot = 0;
        } else if (crit_b.sequence > crit_a.sequence && crit_b.sequence - crit_a.sequence == 1) {
            if (!is_legal_successor_transition(&crit_a, &crit_b)) return ST_INVALID;
            out->control = crit_b;
            out->critical_mirrored = false;
            out->critical_slot = 1;
        } else {
            return ST_INVALID;
        }
    } else if (val_a && !val_b) {
        out->control = crit_a;
        out->critical_mirrored = false;
        out->critical_slot = 0;
    } else if (!val_a && val_b) {
        out->control = crit_b;
        out->critical_mirrored = false;
        out->critical_slot = 1;
    } else {
        return ST_INVALID;
    }

    /* Decode and select progress records */
    out->has_progress = false;
    out->progress_slot = 0;
    memset(&out->progress, 0, sizeof(out->progress));

    if (out->control.phase == ST_PHASE_INHIBIT || out->control.phase == ST_PHASE_RETRY_WAIT) {
        const uint8_t *raw_prog_a = &raw[2 * FRAM_CRITICAL_RECORD_SIZE];
        const uint8_t *raw_prog_b = &raw[2 * FRAM_CRITICAL_RECORD_SIZE + FRAM_PROGRESS_RECORD_SIZE];

        StProgress prog_a, prog_b;
        bool pval_a = validate_progress_record(raw_prog_a, &prog_a, &out->control);
        bool pval_b = validate_progress_record(raw_prog_b, &prog_b, &out->control);

        if (pval_a && pval_b) {
            if (prog_a.sequence == prog_b.sequence) {
                if (memcmp(raw_prog_a, raw_prog_b, FRAM_PROGRESS_RECORD_SIZE) != 0) {
                    return ST_INVALID;
                }
                out->has_progress = true;
                out->progress = prog_a;
                out->progress_slot = 0;
            } else if (prog_a.sequence > prog_b.sequence) {
                if (prog_a.elapsed_ms < prog_b.elapsed_ms) return ST_INVALID;
                out->has_progress = true;
                out->progress = prog_a;
                out->progress_slot = 0;
            } else {
                if (prog_b.elapsed_ms < prog_a.elapsed_ms) return ST_INVALID;
                out->has_progress = true;
                out->progress = prog_b;
                out->progress_slot = 1;
            }
        } else if (pval_a && !pval_b) {
            out->has_progress = true;
            out->progress = prog_a;
            out->progress_slot = 0;
        } else if (!pval_a && pval_b) {
            out->has_progress = true;
            out->progress = prog_b;
            out->progress_slot = 1;
        } else {
            out->has_progress = false;
        }
    }

    return out->critical_mirrored ? ST_OK : ST_RECOVERABLE;
}

/* Compare every saved byte before deriving authority; decoded caller fields are advisory. */
static StResult reload_expected(const StSnapshot *expected, StSnapshot *current) {
    uint8_t raw[FRAM_STARTUP_SNAPSHOT_SIZE];
    if (!FRAM_readData(FRAM_STARTUP_SNAPSHOT_ADDR, raw, sizeof(raw))) return ST_IO;
    if (memcmp(raw, expected->raw, sizeof(raw)) != 0) return ST_CONFLICT;
    return decode_snapshot_raw(raw, current);
}

/* Public st_load: fresh recovery entry, reloads FRAM and invalidates volatile cursor */
StResult st_load(StSnapshot *out) {
    /* Invalidate volatile authority before validation: every public call revokes permits. */
    s_active_cursor.active = false;
    s_active_cursor.token = 0;
    invalidate_permit();
    if (out == NULL) return ST_INVALID;

    uint8_t raw[FRAM_STARTUP_SNAPSHOT_SIZE];
    if (!FRAM_readData(FRAM_STARTUP_SNAPSHOT_ADDR, raw, FRAM_STARTUP_SNAPSHOT_SIZE)) {
        return ST_IO;
    }

    return decode_snapshot_raw(raw, out);
}

/* Public st_repair */
StResult st_repair(const StSnapshot *expected, StSnapshot *out) {
    invalidate_permit();
    if (expected == NULL || out == NULL) return ST_INVALID;

    uint8_t raw_live[FRAM_STARTUP_SNAPSHOT_SIZE];
    if (!FRAM_readData(FRAM_STARTUP_SNAPSHOT_ADDR, raw_live, FRAM_STARTUP_SNAPSHOT_SIZE)) {
        return ST_IO;
    }

    if (memcmp(raw_live, expected->raw, FRAM_STARTUP_SNAPSHOT_SIZE) != 0) {
        return ST_CONFLICT;
    }

    StSnapshot current;
    StResult decode_res = decode_snapshot_raw(raw_live, &current);
    if (decode_res != ST_OK && decode_res != ST_RECOVERABLE) {
        return decode_res;
    }

    if (current.critical_mirrored) {
        *out = current;
        return ST_OK;
    }

    uint16_t dst_addr = (current.critical_slot == 0) ? FRAM_CRITICAL_B_ADDR : FRAM_CRITICAL_A_ADDR;

    uint8_t record_to_copy[FRAM_CRITICAL_RECORD_SIZE];
    memcpy(record_to_copy, &current.raw[current.critical_slot * FRAM_CRITICAL_RECORD_SIZE],
           FRAM_CRITICAL_RECORD_SIZE);
    memcpy(&current.raw[dst_addr - FRAM_STARTUP_SNAPSHOT_ADDR], record_to_copy,
           FRAM_CRITICAL_RECORD_SIZE);

    s_active_cursor.active = false;
    s_active_cursor.token = 0;
    if (!write_critical_record(dst_addr, record_to_copy)) {
        return ST_IO;
    }

    if (!FRAM_readData(FRAM_STARTUP_SNAPSHOT_ADDR, raw_live, FRAM_STARTUP_SNAPSHOT_SIZE)) {
        return ST_IO;
    }

    if (memcmp(raw_live, current.raw, FRAM_STARTUP_SNAPSHOT_SIZE) != 0) return ST_IO;
    StResult post_repair_res = decode_snapshot_raw(raw_live, out);
    if (post_repair_res != ST_OK) {
        return ST_INVALID;
    }
    return ST_OK;
}

/* Public st_start_inhibit */
StResult st_start_inhibit(const StSnapshot *expected, const StClock *clock, StSnapshot *out) {
    invalidate_permit();
    if (expected == NULL || clock == NULL || out == NULL) return ST_INVALID;

    StSnapshot current;
    StResult loaded = reload_expected(expected, &current);
    if (loaded != ST_OK) return loaded;
    expected = &current;

    if (!expected->critical_mirrored) return ST_RECOVERABLE;
    if (expected->control.phase != ST_PHASE_PREPARED) return ST_INVALID;
    if (expected->control.sequence != 0) return ST_INVALID;

    if (expected->control.authorization == 0) {
        return ST_INHIBITED;
    }
    if (expected->control.authorization != 1) return ST_INVALID;

    if (!clock->valid || clock->source < 1 || clock->source > 3) {
        return ST_CLOCK_INVALID;
    }

    uint8_t raw_live[FRAM_STARTUP_SNAPSHOT_SIZE];
    if (!FRAM_readData(FRAM_STARTUP_SNAPSHOT_ADDR, raw_live, FRAM_STARTUP_SNAPSHOT_SIZE)) {
        return ST_IO;
    }

    if (memcmp(raw_live, expected->raw, FRAM_STARTUP_SNAPSHOT_SIZE) != 0) {
        return ST_CONFLICT;
    }

    StControl next_ctrl = {0};
    next_ctrl.arming = expected->control.arming;
    next_ctrl.sequence = 1;
    next_ctrl.reservation = 0;
    next_ctrl.wait = 1;
    next_ctrl.duration_ms = INHIBIT_DURATION_MS;
    next_ctrl.elapsed_ms = 0;
    next_ctrl.reference_ms = clock->epoch_ms;
    next_ctrl.source = clock->source;
    next_ctrl.flags = 0;
    next_ctrl.phase = ST_PHASE_INHIBIT;
    next_ctrl.fast_count = 0;
    next_ctrl.authorization = 1;
    next_ctrl.reference_valid = 1;

    uint8_t successor_rec[FRAM_CRITICAL_RECORD_SIZE];
    encode_critical_record(&next_ctrl, successor_rec);
    memcpy(current.raw, successor_rec, FRAM_CRITICAL_RECORD_SIZE);
    memcpy(&current.raw[FRAM_CRITICAL_B_ADDR - FRAM_STARTUP_SNAPSHOT_ADDR],
           successor_rec, FRAM_CRITICAL_RECORD_SIZE);

    /* Write successor to replica B @ 104, then mirror to replica A @ 8 */
    if (!write_critical_record(FRAM_CRITICAL_B_ADDR, successor_rec)) {
        return ST_IO;
    }
    if (!write_critical_record(FRAM_CRITICAL_A_ADDR, successor_rec)) {
        return ST_IO;
    }

    if (!FRAM_readData(FRAM_STARTUP_SNAPSHOT_ADDR, raw_live, FRAM_STARTUP_SNAPSHOT_SIZE)) {
        return ST_IO;
    }

    if (memcmp(raw_live, current.raw, FRAM_STARTUP_SNAPSHOT_SIZE) != 0) {
        s_active_cursor.active = false;
        s_active_cursor.token = 0;
        return ST_IO;
    }
    StResult res = decode_snapshot_raw(raw_live, out);
    if (res != ST_OK) return ST_INVALID;
    return ST_OK;
}

/* Public st_wait_open */
StResult st_wait_open(const StSnapshot *snapshot, const StClock *clock, StEntry entry,
                      StWaitCursor *cursor) {
    invalidate_permit();
    s_active_cursor.active = false;
    s_active_cursor.token = 0;
    if (cursor != NULL) cursor->token = 0;
    if (snapshot == NULL || clock == NULL || cursor == NULL) return ST_INVALID;
    if (!clock->valid || clock->source < 1 || clock->source > 3) return ST_CLOCK_INVALID;
    if (entry != ST_NO_OLD_TIME_CREDIT && entry != ST_BOOT_CONTINUITY_PROVEN) return ST_INVALID;

    StSnapshot current;
    StResult loaded = reload_expected(snapshot, &current);
    if (loaded != ST_OK && loaded != ST_RECOVERABLE) return loaded;
    snapshot = &current;

    if (snapshot->control.phase != ST_PHASE_INHIBIT &&
        snapshot->control.phase != ST_PHASE_RETRY_WAIT) {
        return ST_INVALID;
    }

    uint64_t saved_elapsed = 0;
    uint64_t saved_ref = 0;
    uint32_t saved_source = 0;
    uint8_t saved_ref_valid = 0;

    if (snapshot->has_progress) {
        saved_elapsed = snapshot->progress.elapsed_ms;
        saved_ref = snapshot->progress.reference_ms;
        saved_source = snapshot->progress.source;
        saved_ref_valid = snapshot->progress.reference_valid;
    } else {
        saved_elapsed = snapshot->control.elapsed_ms;
        saved_ref = snapshot->control.reference_ms;
        saved_source = snapshot->control.source;
        saved_ref_valid = snapshot->control.reference_valid;
    }

    uint64_t measured_elapsed = saved_elapsed;
    bool continuity_proven = entry == ST_BOOT_CONTINUITY_PROVEN &&
        saved_ref_valid == 1 && clock->source == saved_source && clock->epoch_ms >= saved_ref;
    if (continuity_proven) {
        uint64_t delta = clock->epoch_ms - saved_ref;
        if (measured_elapsed + delta < measured_elapsed) {
            measured_elapsed = snapshot->control.duration_ms;
        } else {
            measured_elapsed += delta;
            if (measured_elapsed > snapshot->control.duration_ms) {
                measured_elapsed = snapshot->control.duration_ms;
            }
        }
    }

    if (s_cursor_token_counter == UINT64_MAX) return ST_EXHAUSTED;
    uint64_t tok = s_cursor_token_counter++;

    s_active_cursor.token = tok;
    s_active_cursor.active = true;
    memcpy(s_active_cursor.raw_snapshot, snapshot->raw, FRAM_STARTUP_SNAPSHOT_SIZE);
    s_active_cursor.arming = snapshot->control.arming;
    s_active_cursor.wait_id = snapshot->control.wait;
    s_active_cursor.duration_ms = snapshot->control.duration_ms;
    s_active_cursor.baseline_elapsed_ms = snapshot->control.elapsed_ms;
    s_active_cursor.durable_elapsed_ms = saved_elapsed;
    s_active_cursor.durable_reference_ms = saved_ref;
    s_active_cursor.durable_source = saved_source;
    s_active_cursor.durable_ref_valid = saved_ref_valid;
    s_active_cursor.last_epoch_ms = clock->epoch_ms;
    s_active_cursor.last_source = clock->source;
    s_active_cursor.last_revision = clock->revision;
    s_active_cursor.measured_elapsed_ms = measured_elapsed;
    s_active_cursor.entry_checkpoint_pending = !continuity_proven;
    s_active_cursor.entry_elapsed_ms = saved_elapsed;
    s_active_cursor.entry_clock = *clock;

    cursor->token = tok;
    return ST_OK;
}

/* Public st_wait_measure */
StResult st_wait_measure(const StSnapshot *snapshot, StWaitCursor *cursor,
                         const StClock *clock, uint64_t *elapsed_ms) {
    invalidate_permit();
    if (elapsed_ms != NULL) *elapsed_ms = 0;
    if (snapshot == NULL || cursor == NULL || clock == NULL || elapsed_ms == NULL) {
        return ST_INVALID;
    }

    if (!s_active_cursor.active || cursor->token == 0 || cursor->token != s_active_cursor.token) {
        return ST_INVALID;
    }

    /* Failure must never expose an unsaved or caller-supplied elapsed value. */
    *elapsed_ms = s_active_cursor.durable_elapsed_ms;
    if (memcmp(snapshot->raw, s_active_cursor.raw_snapshot, FRAM_STARTUP_SNAPSHOT_SIZE) != 0) {
        return ST_CONFLICT;
    }
    StSnapshot current;
    StResult loaded = reload_expected(snapshot, &current);
    if (loaded != ST_OK && loaded != ST_RECOVERABLE) return loaded;

    if (!clock->valid || clock->source < 1 || clock->source > 3) {
        s_active_cursor.active = false;
        s_active_cursor.token = 0;
        cursor->token = 0;
        return ST_CLOCK_INVALID;
    }

    if (clock->epoch_ms < s_active_cursor.last_epoch_ms ||
        clock->source != s_active_cursor.last_source ||
        clock->revision != s_active_cursor.last_revision) {
        s_active_cursor.active = false;
        s_active_cursor.token = 0;
        cursor->token = 0;
        return ST_REANCHOR_REQUIRED;
    }

    uint64_t delta = clock->epoch_ms - s_active_cursor.last_epoch_ms;
    if (s_active_cursor.measured_elapsed_ms + delta < s_active_cursor.measured_elapsed_ms) {
        s_active_cursor.measured_elapsed_ms = s_active_cursor.duration_ms;
    } else {
        s_active_cursor.measured_elapsed_ms += delta;
        if (s_active_cursor.measured_elapsed_ms > s_active_cursor.duration_ms) {
            s_active_cursor.measured_elapsed_ms = s_active_cursor.duration_ms;
        }
    }
    s_active_cursor.last_epoch_ms = clock->epoch_ms;
    *elapsed_ms = s_active_cursor.measured_elapsed_ms;
    return ST_OK;
}

/* Public st_checkpoint */
StResult st_checkpoint(const StSnapshot *snapshot, StWaitCursor *cursor,
                       const StClock *clock, StSnapshot *out) {
    invalidate_permit();
    if (snapshot == NULL || cursor == NULL || clock == NULL || out == NULL) {
        return ST_INVALID;
    }

    if (!s_active_cursor.active || cursor->token == 0 || cursor->token != s_active_cursor.token) {
        return ST_INVALID;
    }

    if (memcmp(snapshot->raw, s_active_cursor.raw_snapshot, FRAM_STARTUP_SNAPSHOT_SIZE) != 0) {
        return ST_CONFLICT;
    }

    if (!clock->valid || clock->source < 1 || clock->source > 3) {
        s_active_cursor.active = false;
        s_active_cursor.token = 0;
        cursor->token = 0;
        return ST_CLOCK_INVALID;
    }

    if (clock->epoch_ms < s_active_cursor.last_epoch_ms ||
        clock->source != s_active_cursor.last_source ||
        clock->revision != s_active_cursor.last_revision) {
        s_active_cursor.active = false;
        s_active_cursor.token = 0;
        cursor->token = 0;
        return ST_REANCHOR_REQUIRED;
    }

    uint64_t delta = clock->epoch_ms - s_active_cursor.last_epoch_ms;
    uint64_t new_elapsed = s_active_cursor.measured_elapsed_ms;
    if (new_elapsed + delta < new_elapsed) {
        new_elapsed = s_active_cursor.duration_ms;
    } else {
        new_elapsed += delta;
        if (new_elapsed > s_active_cursor.duration_ms) {
            new_elapsed = s_active_cursor.duration_ms;
        }
    }

    uint8_t raw_live[FRAM_STARTUP_SNAPSHOT_SIZE];
    if (!FRAM_readData(FRAM_STARTUP_SNAPSHOT_ADDR, raw_live, FRAM_STARTUP_SNAPSHOT_SIZE)) {
        return ST_IO;
    }

    if (memcmp(raw_live, snapshot->raw, FRAM_STARTUP_SNAPSHOT_SIZE) != 0) {
        return ST_CONFLICT;
    }

    StSnapshot current;
    StResult loaded = decode_snapshot_raw(raw_live, &current);
    if (loaded != ST_OK) return loaded;
    snapshot = &current;

    uint64_t new_seq = 1;
    uint16_t dst_addr = FRAM_PROGRESS_A_ADDR;
    if (snapshot->has_progress) {
        if (snapshot->progress.sequence == UINT64_MAX) {
            return ST_EXHAUSTED;
        }
        new_seq = snapshot->progress.sequence + 1;
        dst_addr = (snapshot->progress_slot == 0) ? FRAM_PROGRESS_B_ADDR : FRAM_PROGRESS_A_ADDR;
    }

    StProgress new_prog = {0};
    new_prog.arming = s_active_cursor.arming;
    new_prog.wait = s_active_cursor.wait_id;
    new_prog.sequence = new_seq;
    /* The first no-credit checkpoint records entry evidence, not the later sample. */
    bool entry_checkpoint = s_active_cursor.entry_checkpoint_pending;
    new_prog.elapsed_ms = entry_checkpoint ? s_active_cursor.entry_elapsed_ms : new_elapsed;
    new_prog.reference_ms = entry_checkpoint ? s_active_cursor.entry_clock.epoch_ms : clock->epoch_ms;
    new_prog.source = entry_checkpoint ? s_active_cursor.entry_clock.source : clock->source;
    new_prog.reference_valid = 1;

    uint8_t prog_bytes[FRAM_PROGRESS_RECORD_SIZE];
    encode_progress_record(&new_prog, prog_bytes);
    memcpy(&current.raw[dst_addr - FRAM_STARTUP_SNAPSHOT_ADDR], prog_bytes,
           FRAM_PROGRESS_RECORD_SIZE);

    if (!write_progress_record(dst_addr, prog_bytes)) {
        s_active_cursor.active = false;
        s_active_cursor.token = 0;
        cursor->token = 0;
        return ST_IO;
    }

    if (!FRAM_readData(FRAM_STARTUP_SNAPSHOT_ADDR, raw_live, FRAM_STARTUP_SNAPSHOT_SIZE) ||
        memcmp(raw_live, current.raw, FRAM_STARTUP_SNAPSHOT_SIZE) != 0) {
        s_active_cursor.active = false;
        s_active_cursor.token = 0;
        cursor->token = 0;
        return ST_IO;
    }

    StResult res = decode_snapshot_raw(raw_live, out);
    if (res != ST_OK) {
        s_active_cursor.active = false;
        s_active_cursor.token = 0;
        cursor->token = 0;
        return ST_INVALID;
    }

    /* Atomically rebind cursor to returned snapshot and coherent durable elapsed/reference */
    memcpy(s_active_cursor.raw_snapshot, out->raw, FRAM_STARTUP_SNAPSHOT_SIZE);
    s_active_cursor.durable_elapsed_ms = out->progress.elapsed_ms;
    s_active_cursor.durable_reference_ms = out->progress.reference_ms;
    s_active_cursor.durable_source = out->progress.source;
    s_active_cursor.durable_ref_valid = out->progress.reference_valid;
    s_active_cursor.entry_checkpoint_pending = false;
    /* Keep current-boot measurement separate from the older durable entry barrier. */
    s_active_cursor.measured_elapsed_ms = new_elapsed;
    s_active_cursor.last_epoch_ms = clock->epoch_ms;
    s_active_cursor.last_source = clock->source;
    s_active_cursor.last_revision = clock->revision;

    return ST_OK;
}

/* Public st_finish_inhibit */
StResult st_finish_inhibit(const StSnapshot *snapshot, StWaitCursor *cursor,
                           const StClock *clock, StSnapshot *out) {
    invalidate_permit();
    if (snapshot == NULL || cursor == NULL || clock == NULL || out == NULL) {
        return ST_INVALID;
    }

    StSnapshot current;
    StResult loaded = reload_expected(snapshot, &current);
    if (loaded != ST_OK) return loaded;
    snapshot = &current;
    if (snapshot->control.phase != ST_PHASE_INHIBIT) return ST_INVALID;

    if (!s_active_cursor.active || cursor->token == 0 || cursor->token != s_active_cursor.token) {
        return ST_INVALID;
    }

    if (memcmp(snapshot->raw, s_active_cursor.raw_snapshot, FRAM_STARTUP_SNAPSHOT_SIZE) != 0) {
        return ST_CONFLICT;
    }

    if (!clock->valid || clock->source < 1 || clock->source > 3) {
        s_active_cursor.active = false;
        s_active_cursor.token = 0;
        cursor->token = 0;
        return ST_CLOCK_INVALID;
    }

    if (clock->epoch_ms < s_active_cursor.last_epoch_ms ||
        clock->source != s_active_cursor.last_source ||
        clock->revision != s_active_cursor.last_revision) {
        s_active_cursor.active = false;
        s_active_cursor.token = 0;
        cursor->token = 0;
        return ST_REANCHOR_REQUIRED;
    }

    uint64_t delta = clock->epoch_ms - s_active_cursor.last_epoch_ms;
    uint64_t new_elapsed = s_active_cursor.measured_elapsed_ms;
    if (new_elapsed + delta < new_elapsed) {
        new_elapsed = s_active_cursor.duration_ms;
    } else {
        new_elapsed += delta;
        if (new_elapsed > s_active_cursor.duration_ms) {
            new_elapsed = s_active_cursor.duration_ms;
        }
    }

    if (new_elapsed < snapshot->control.duration_ms) {
        /* Not due yet: update measured elapsed up to clock, return ST_NOT_DUE without writes */
        s_active_cursor.measured_elapsed_ms = new_elapsed;
        s_active_cursor.last_epoch_ms = clock->epoch_ms;
        return ST_NOT_DUE;
    }

    uint8_t raw_live[FRAM_STARTUP_SNAPSHOT_SIZE];
    if (!FRAM_readData(FRAM_STARTUP_SNAPSHOT_ADDR, raw_live, FRAM_STARTUP_SNAPSHOT_SIZE)) {
        return ST_IO;
    }

    if (memcmp(raw_live, snapshot->raw, FRAM_STARTUP_SNAPSHOT_SIZE) != 0) {
        return ST_CONFLICT;
    }

    /* A pending entry barrier must precede the forced current-time checkpoint. */
    bool entry_checkpoint = s_active_cursor.entry_checkpoint_pending;
    StSnapshot checkpoint_snap;
    StResult cp_res = st_checkpoint(snapshot, cursor, clock, &checkpoint_snap);
    if (cp_res != ST_OK) return cp_res;
    if (entry_checkpoint) {
        cp_res = st_checkpoint(&checkpoint_snap, cursor, clock, &checkpoint_snap);
        if (cp_res != ST_OK) {
            /* The caller still holds the pre-barrier snapshot: require fresh recovery. */
            s_active_cursor.active = false;
            s_active_cursor.token = 0;
            cursor->token = 0;
            return cp_res;
        }
    }

    /* Construct critical successor: INHIBIT -> ELIGIBLE (seq 2, flags=1, clear wait) */
    StControl next_ctrl = {0};
    next_ctrl.arming = checkpoint_snap.control.arming;
    next_ctrl.sequence = 2;
    next_ctrl.reservation = 0;
    next_ctrl.wait = 0;
    next_ctrl.duration_ms = 0;
    next_ctrl.elapsed_ms = 0;
    next_ctrl.reference_ms = 0;
    next_ctrl.source = 0;
    next_ctrl.flags = ST_FLAG_INHIBIT_DONE;
    next_ctrl.phase = ST_PHASE_ELIGIBLE;
    next_ctrl.fast_count = 0;
    next_ctrl.authorization = 1;
    next_ctrl.reference_valid = 0;

    uint8_t successor_rec[FRAM_CRITICAL_RECORD_SIZE];
    encode_critical_record(&next_ctrl, successor_rec);
    memcpy(checkpoint_snap.raw, successor_rec, FRAM_CRITICAL_RECORD_SIZE);
    memcpy(&checkpoint_snap.raw[FRAM_CRITICAL_B_ADDR - FRAM_STARTUP_SNAPSHOT_ADDR],
           successor_rec, FRAM_CRITICAL_RECORD_SIZE);

    /* Write successor to replica B @ 104, then mirror to replica A @ 8 */
    if (!write_critical_record(FRAM_CRITICAL_B_ADDR, successor_rec)) {
        s_active_cursor.active = false;
        s_active_cursor.token = 0;
        cursor->token = 0;
        return ST_IO;
    }
    if (!write_critical_record(FRAM_CRITICAL_A_ADDR, successor_rec)) {
        s_active_cursor.active = false;
        s_active_cursor.token = 0;
        cursor->token = 0;
        return ST_IO;
    }

    /* Invalidate wait cursor now that wait has ended */
    s_active_cursor.active = false;
    s_active_cursor.token = 0;
    cursor->token = 0;

    if (!FRAM_readData(FRAM_STARTUP_SNAPSHOT_ADDR, raw_live, FRAM_STARTUP_SNAPSHOT_SIZE)) {
        return ST_IO;
    }

    if (memcmp(raw_live, checkpoint_snap.raw, FRAM_STARTUP_SNAPSHOT_SIZE) != 0) return ST_IO;
    StResult final_res = decode_snapshot_raw(raw_live, out);
    if (final_res != ST_OK) return ST_INVALID;
    return ST_OK;
}

/* Commit an already validated adjacent critical successor, successor first. */
static StResult commit_control(const StSnapshot *current, const StControl *next,
                               StSnapshot *out) {
    if (current->control.sequence == UINT64_MAX) return ST_EXHAUSTED;
    uint8_t rec[FRAM_CRITICAL_RECORD_SIZE];
    encode_critical_record(next, rec);
    uint8_t expected[FRAM_STARTUP_SNAPSHOT_SIZE];
    memcpy(expected, current->raw, sizeof(expected));
    memcpy(expected, rec, FRAM_CRITICAL_RECORD_SIZE);
    memcpy(&expected[FRAM_CRITICAL_B_ADDR - FRAM_STARTUP_SNAPSHOT_ADDR], rec,
           FRAM_CRITICAL_RECORD_SIZE);
    if (!write_critical_record(FRAM_CRITICAL_B_ADDR, rec) ||
        !write_critical_record(FRAM_CRITICAL_A_ADDR, rec)) return ST_IO;
    uint8_t raw[FRAM_STARTUP_SNAPSHOT_SIZE];
    if (!FRAM_readData(FRAM_STARTUP_SNAPSHOT_ADDR, raw, sizeof(raw))) return ST_IO;
    if (memcmp(raw, expected, sizeof(raw)) != 0) {
        s_active_cursor.active = false;
        s_active_cursor.token = 0;
        return ST_IO;
    }
    StResult result = decode_snapshot_raw(raw, out);
    return result == ST_OK ? ST_OK : (result == ST_RECOVERABLE ? ST_IO : ST_INVALID);
}

StResult st_reserve(const StSnapshot *snapshot, StWaitCursor *cursor,
                    const StClock *clock, StSnapshot *out, StOnPermit *permit) {
    invalidate_permit();
    if (snapshot == NULL || out == NULL || permit == NULL) return ST_INVALID;
    permit->token = 0;
    StSnapshot current;
    StResult r = reload_expected(snapshot, &current);
    if (r != ST_OK) return r;
    if (!current.critical_mirrored) return ST_RECOVERABLE;
    uint8_t phase = current.control.phase;
    if (phase != ST_PHASE_ELIGIBLE && phase != ST_PHASE_RETRY_WAIT) return ST_INVALID;
    if (current.control.sequence > UINT64_MAX - 4U ||
        s_permit_token_counter == UINT64_MAX) return ST_EXHAUSTED;

    if (phase == ST_PHASE_RETRY_WAIT) {
        if (cursor == NULL || clock == NULL) return ST_INVALID;
        uint64_t elapsed = 0;
        r = st_wait_measure(&current, cursor, clock, &elapsed);
        if (r != ST_OK) return r;
        if (elapsed < current.control.duration_ms) return ST_NOT_DUE;
        bool entry_barrier = s_active_cursor.entry_checkpoint_pending;
        StSnapshot checked;
        r = st_checkpoint(&current, cursor, clock, &checked);
        if (r != ST_OK) return r;
        if (entry_barrier) {
            r = st_checkpoint(&checked, cursor, clock, &checked);
            if (r != ST_OK) { s_active_cursor.active = false; cursor->token = 0; return r; }
        }
        current = checked;
        s_active_cursor.active = false;
        s_active_cursor.token = 0;
        cursor->token = 0;
    } else if (cursor != NULL) {
        return ST_INVALID;
    }

    StControl next = current.control;
    next.sequence++;
    next.reservation = next.sequence;
    if (next.fast_count < 7) next.fast_count++;
    next.phase = ST_PHASE_RESERVED;
    next.wait = next.duration_ms = next.elapsed_ms = next.reference_ms = 0;
    next.source = 0;
    next.reference_valid = 0;
    r = commit_control(&current, &next, out);
    if (r != ST_OK) return r;
    s_active_permit_token = s_permit_token_counter++;
    s_active_permit_available = 1;
    memcpy(s_active_permit_raw, out->raw, sizeof(s_active_permit_raw));
    permit->token = s_active_permit_token;
    return ST_OK;
}

StResult st_take_on_permit(const StSnapshot *snapshot, StOnPermit *permit) {
    if (permit == NULL) {
        invalidate_permit();
        return ST_INVALID;
    }
    uint64_t token = permit->token;
    permit->token = 0;
    if (token == 0 || !s_active_permit_available || token != s_active_permit_token) {
        invalidate_permit();
        return ST_INVALID;
    }
    s_active_permit_available = 0;
    s_active_permit_token = 0;
    bool matches_permit_snapshot = snapshot != NULL &&
        memcmp(snapshot->raw, s_active_permit_raw, sizeof(s_active_permit_raw)) == 0;
    memset(s_active_permit_raw, 0, sizeof(s_active_permit_raw));
    StSnapshot current;
    StResult r = !matches_permit_snapshot ? ST_CONFLICT : reload_expected(snapshot, &current);
    if (r != ST_OK || !current.critical_mirrored || current.control.phase != ST_PHASE_RESERVED) {
        return r == ST_OK ? ST_INVALID : r;
    }
    return ST_OK;
}

StResult st_after_off(const StSnapshot *snapshot, const StClock *clock, StSnapshot *out) {
    invalidate_permit();
    if (snapshot == NULL || clock == NULL || out == NULL) return ST_INVALID;
    StSnapshot current;
    StResult r = reload_expected(snapshot, &current);
    if (r != ST_OK) return r;
    if (!current.critical_mirrored) return ST_RECOVERABLE;
    if (current.control.phase != ST_PHASE_RESERVED) return ST_INVALID;
    if (current.control.sequence == UINT64_MAX) return ST_EXHAUSTED;
    if (!clock->valid || clock->source < 1 || clock->source > 3) return ST_CLOCK_INVALID;
    StControl next = current.control;
    next.sequence++;
    next.wait = next.sequence;
    next.duration_ms = next.fast_count < 7 ? RETRY_WAIT_SHORT_MS : RETRY_WAIT_LONG_MS;
    next.elapsed_ms = 0;
    next.reference_ms = clock->epoch_ms;
    next.source = clock->source;
    next.reference_valid = 1;
    next.phase = ST_PHASE_RETRY_WAIT;
    return commit_control(&current, &next, out);
}

StResult st_retire(const StSnapshot *snapshot, StSnapshot *out) {
    invalidate_permit();
    if (snapshot == NULL || out == NULL) return ST_INVALID;
    StSnapshot current;
    StResult r = reload_expected(snapshot, &current);
    if (r != ST_OK) return r;
    if (!current.critical_mirrored) return ST_RECOVERABLE;
    if (current.control.phase != ST_PHASE_ELIGIBLE && current.control.phase != ST_PHASE_RETRY_WAIT) return ST_INVALID;
    if (current.control.sequence >= UINT64_MAX - 1U) return ST_EXHAUSTED;
    StControl next = current.control;
    next.sequence++;
    next.phase = ST_PHASE_RETIRED;
    next.wait = next.duration_ms = next.elapsed_ms = next.reference_ms = 0;
    next.source = 0;
    next.reference_valid = 0;
    return commit_control(&current, &next, out);
}

StResult st_complete(const StSnapshot *snapshot, StSnapshot *out) {
    invalidate_permit();
    if (snapshot == NULL || out == NULL) return ST_INVALID;
    StSnapshot current;
    StResult r = reload_expected(snapshot, &current);
    if (r != ST_OK) return r;
    if (!current.critical_mirrored) return ST_RECOVERABLE;
    if (current.control.phase != ST_PHASE_RETIRED) return ST_INVALID;
    if (current.control.sequence == UINT64_MAX) return ST_EXHAUSTED;
    StControl next = current.control;
    next.sequence++;
    next.phase = ST_PHASE_COMPLETE;
    return commit_control(&current, &next, out);
}
