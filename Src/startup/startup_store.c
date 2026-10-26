/*
 * startup_store.c
 *
 * The only Startup persistence code: one 20-byte SSP1 record codec and the
 * two-slot load/save that uses it. Byte offsets are fixed by the shared wire
 * contract and are encoded explicitly, never by copying a C struct.
 *
 * Failure model: a save only rewrites the inactive slot and commits last, so an
 * interrupted save leaves the previous record usable and load returns either
 * the complete old or the complete new record. This assumes the previous slot
 * is intact and the device behaves electrically as specified; it is not an
 * arbitrary-corruption or physical-actuation atomicity claim.
 */

#include "startup/startup_store.h"

#include <string.h>

#include "FRAM/MB85RS256B.h"

enum {
    STORE_OFFSET_TAG = 0,        /* ASCII "SSP1" */
    STORE_OFFSET_SEQUENCE = 4,   /* u32 little endian, 1 and up */
    STORE_OFFSET_STATE = 8,      /* MCUState value */
    STORE_OFFSET_ATTEMPTS = 9,   /* 0..STARTUP_ATTEMPT_LIMIT */
    STORE_OFFSET_FLAGS = 10,     /* bit 0 armed */
    STORE_OFFSET_REMAINING = 11, /* u32 little endian milliseconds */
    STORE_OFFSET_CRC = 15,       /* u32 little endian over bytes 0..14 */
    STORE_OFFSET_COMMIT = 19
};

#define STORE_FLAG_ARMED 0x01u
#define STORE_COMMIT_MARKER 0xA5u
#define STORE_COMMIT_INVALID 0x00u

static const uint8_t store_tag[4] = {'S', 'S', 'P', '1'};

/* Slot chosen by the last successful load, and the sequence it carried. It stays
 * known after a failed save on purpose: Startup faults on any uncertain result, a
 * retry re-invalidates the same inactive slot, and the previous slot was never
 * written. */
static struct {
    bool known;
    uint8_t slot; /* 0 = A (8), 1 = B (28) */
    uint32_t sequence;
} s_active;

/* Reflected CRC32 (0xEDB88320), init/final xor 0xFFFFFFFF: zlib.crc32 compatible. */
static uint32_t store_crc32(const uint8_t *data, size_t size) {
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < size; ++i) {
        crc ^= data[i];
        for (unsigned bit = 0; bit < 8; ++bit) {
            crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1u)));
        }
    }
    return crc ^ 0xFFFFFFFFu;
}

static uint32_t store_read_u32(const uint8_t *bytes) {
    return (uint32_t)bytes[0] | ((uint32_t)bytes[1] << 8) |
           ((uint32_t)bytes[2] << 16) | ((uint32_t)bytes[3] << 24);
}

static void store_write_u32(uint8_t *bytes, uint32_t value) {
    bytes[0] = (uint8_t)value;
    bytes[1] = (uint8_t)(value >> 8);
    bytes[2] = (uint8_t)(value >> 16);
    bytes[3] = (uint8_t)(value >> 24);
}

uint32_t startup_store_retry_ms(uint8_t attempts_used) {
    return attempts_used < STARTUP_ATTEMPT_LIMIT ? STARTUP_RETRY_MS : STARTUP_WEEKLY_MS;
}

bool startup_store_state_is_valid(const StartupStoreState *state) {
    if (state->attempts_used > STARTUP_ATTEMPT_LIMIT) return false;
    switch ((MCUState)state->resume_state) {
    case INITIAL_CHECKS:
        return state->attempts_used == 0 && state->remaining_ms <= STARTUP_INHIBIT_MS;
    case WAIT_POWER_ON:
        return state->armed && state->remaining_ms == 0;
    case WAITING_FOR_UPLINK:
        return state->armed && state->attempts_used >= 1 &&
               state->remaining_ms <= startup_store_retry_ms(state->attempts_used);
    case SAT_TO_GROUND_1:
    case MAIN_OPS:
        return state->armed && state->remaining_ms == 0;
    case GROUND_TO_SAT_1:
    case SAT_TO_GROUND_2:
    case DIAGNOSTICS_MODE:
    default:
        /* Transient handshake/diagnostics states are never persisted. */
        return false;
    }
}

static void store_encode(const StartupStoreState *state, uint32_t sequence, uint8_t raw[FRAM_STARTUP_SLOT_SIZE]) {
    memcpy(raw + STORE_OFFSET_TAG, store_tag, sizeof(store_tag));
    store_write_u32(raw + STORE_OFFSET_SEQUENCE, sequence);
    raw[STORE_OFFSET_STATE] = state->resume_state;
    raw[STORE_OFFSET_ATTEMPTS] = state->attempts_used;
    raw[STORE_OFFSET_FLAGS] = state->armed ? STORE_FLAG_ARMED : 0u;
    store_write_u32(raw + STORE_OFFSET_REMAINING, state->remaining_ms);
    store_write_u32(raw + STORE_OFFSET_CRC, store_crc32(raw, STORE_OFFSET_CRC));
    raw[STORE_OFFSET_COMMIT] = STORE_COMMIT_MARKER;
}

static bool store_decode(const uint8_t raw[FRAM_STARTUP_SLOT_SIZE], StartupStoreState *out, uint32_t *sequence) {
    if (raw[STORE_OFFSET_COMMIT] != STORE_COMMIT_MARKER) return false;
    if (memcmp(raw + STORE_OFFSET_TAG, store_tag, sizeof(store_tag)) != 0) return false;
    if (store_read_u32(raw + STORE_OFFSET_CRC) != store_crc32(raw, STORE_OFFSET_CRC)) return false;
    if ((raw[STORE_OFFSET_FLAGS] & (uint8_t)~STORE_FLAG_ARMED) != 0) return false;
    uint32_t saved_sequence = store_read_u32(raw + STORE_OFFSET_SEQUENCE);
    if (saved_sequence == 0) return false;
    StartupStoreState state = {
        .resume_state = raw[STORE_OFFSET_STATE],
        .attempts_used = raw[STORE_OFFSET_ATTEMPTS],
        .armed = (raw[STORE_OFFSET_FLAGS] & STORE_FLAG_ARMED) != 0,
        .remaining_ms = store_read_u32(raw + STORE_OFFSET_REMAINING),
    };
    if (!startup_store_state_is_valid(&state)) return false;
    *out = state;
    *sequence = saved_sequence;
    return true;
}

static StartupStoreResult store_read(uint16_t address, uint8_t *out, uint16_t size) {
    return FRAM_readData(address, out, size) ? STARTUP_STORE_OK : STARTUP_STORE_IO;
}

static StartupStoreResult store_write(uint16_t address, const uint8_t *data, uint16_t size) {
    /* Write enable is required per operation; a failed write-disable leaves the
     * device state uncertain, so it is part of the result. */
    if (!FRAM_writeEnable()) return STARTUP_STORE_IO;
    bool written = FRAM_writeData(address, (uint8_t *)data, size);
    bool disabled = FRAM_writeDisable();
    if (!written || !disabled) return STARTUP_STORE_IO;
    return STARTUP_STORE_OK;
}

static StartupStoreResult store_verify(uint16_t address, const uint8_t *expected, uint16_t size) {
    uint8_t readback[FRAM_STARTUP_SLOT_SIZE];
    if (store_read(address, readback, size) != STARTUP_STORE_OK) return STARTUP_STORE_IO;
    return memcmp(readback, expected, size) == 0 ? STARTUP_STORE_OK : STARTUP_STORE_IO;
}

static uint16_t store_slot_address(uint8_t slot) {
    return slot == 0 ? FRAM_STARTUP_SLOT_A_ADDR : FRAM_STARTUP_SLOT_B_ADDR;
}

StartupStoreResult startup_store_load(StartupStoreState *out) {
    uint8_t raw_a[FRAM_STARTUP_SLOT_SIZE];
    uint8_t raw_b[FRAM_STARTUP_SLOT_SIZE];
    StartupStoreState state_a = {0};
    StartupStoreState state_b = {0};
    uint32_t sequence_a = 0;
    uint32_t sequence_b = 0;

    s_active.known = false;
    if (store_read(FRAM_STARTUP_SLOT_A_ADDR, raw_a, sizeof(raw_a)) != STARTUP_STORE_OK ||
        store_read(FRAM_STARTUP_SLOT_B_ADDR, raw_b, sizeof(raw_b)) != STARTUP_STORE_OK) {
        return STARTUP_STORE_IO;
    }

    bool valid_a = store_decode(raw_a, &state_a, &sequence_a);
    bool valid_b = store_decode(raw_b, &state_b, &sequence_b);
    uint8_t selected;
    if (valid_a && valid_b) {
        if (sequence_a == sequence_b) {
            /* A repeated sequence is only meaningful when the slots agree exactly. */
            if (memcmp(raw_a, raw_b, sizeof(raw_a)) != 0) return STARTUP_STORE_INVALID;
            selected = 0;
        } else {
            selected = sequence_a > sequence_b ? 0 : 1;
        }
    } else if (valid_a) {
        selected = 0;
    } else if (valid_b) {
        selected = 1;
    } else {
        return STARTUP_STORE_INVALID;
    }

    *out = selected == 0 ? state_a : state_b;
    s_active.known = true;
    s_active.slot = selected;
    s_active.sequence = selected == 0 ? sequence_a : sequence_b;
    return STARTUP_STORE_OK;
}

StartupStoreResult startup_store_save(const StartupStoreState *state) {
    if (!s_active.known) return STARTUP_STORE_IO; /* no load decided the target slot */
    if (!startup_store_state_is_valid(state)) return STARTUP_STORE_INVALID;
    if (s_active.sequence == UINT32_MAX) return STARTUP_STORE_EXHAUSTED;

    uint8_t target = s_active.slot == 0 ? 1 : 0;
    uint16_t address = store_slot_address(target);
    uint8_t raw[FRAM_STARTUP_SLOT_SIZE];
    store_encode(state, s_active.sequence + 1, raw);

    /* 1. Make the target uncommitted and verify it, so an interruption from here
     *    on leaves the previous slot as the only complete record. */
    uint8_t uncommitted = STORE_COMMIT_INVALID;
    if (store_write(address + STORE_OFFSET_COMMIT, &uncommitted, 1) != STARTUP_STORE_OK) return STARTUP_STORE_IO;
    if (store_verify(address + STORE_OFFSET_COMMIT, &uncommitted, 1) != STARTUP_STORE_OK) return STARTUP_STORE_IO;

    /* 2. Write and verify the exact payload bytes (tag, sequence, fields, CRC). */
    if (store_write(address, raw, STORE_OFFSET_COMMIT) != STARTUP_STORE_OK) return STARTUP_STORE_IO;
    if (store_verify(address, raw, STORE_OFFSET_COMMIT) != STARTUP_STORE_OK) return STARTUP_STORE_IO;

    /* 3. Commit last and verify the complete record before reporting success. */
    if (store_write(address + STORE_OFFSET_COMMIT, &raw[STORE_OFFSET_COMMIT], 1) != STARTUP_STORE_OK) return STARTUP_STORE_IO;
    if (store_verify(address, raw, sizeof(raw)) != STARTUP_STORE_OK) return STARTUP_STORE_IO;

    s_active.slot = target;
    s_active.sequence += 1;
    return STARTUP_STORE_OK;
}
