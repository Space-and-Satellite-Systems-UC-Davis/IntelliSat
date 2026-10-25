#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/*
 * Independent SSP1 wire fixtures: a second, deliberately separate
 * implementation of the 20-byte record used to state expected bytes and to
 * decode media, so a mistake in the production encoder cannot make a test
 * agree with itself. The literal in CONTRACT-VECTOR.json anchors both.
 */

enum {
    SSP1_TAG = 0,
    SSP1_SEQUENCE = 4,
    SSP1_STATE = 8,
    SSP1_ATTEMPTS = 9,
    SSP1_FLAGS = 10,
    SSP1_REMAINING = 11,
    SSP1_CRC = 15,
    SSP1_COMMIT = 19,
    SSP1_SLOT_SIZE = 20,
    SSP1_IMAGE_SIZE = 48,
    SSP1_ADDRESS = 8,
};

typedef struct {
    uint32_t sequence;
    uint8_t state;
    uint8_t attempts;
    uint8_t flags;
    uint32_t remaining;
    bool committed;
} Ssp1Record;

static void ssp1_le(uint8_t *out, uint32_t value, size_t size) {
    for (size_t i = 0; i < size && i < 4; ++i) out[i] = (uint8_t)(value >> (8 * i));
}

static uint32_t ssp1_le_read(const uint8_t *in, size_t size) {
    uint32_t value = 0;
    for (size_t i = 0; i < size && i < 4; ++i) value |= (uint32_t)in[i] << (8 * i);
    return value;
}

static uint32_t ssp1_crc(const uint8_t *data, size_t size) {
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < size; ++i) {
        crc ^= data[i];
        for (unsigned bit = 0; bit < 8; ++bit) crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1u)));
    }
    return crc ^ 0xFFFFFFFFu;
}

/* Build one committed slot from fields. */
static void ssp1_build(uint8_t slot[SSP1_SLOT_SIZE], uint32_t sequence, uint8_t state,
                       uint8_t attempts, bool armed, uint32_t remaining) {
    memset(slot, 0, SSP1_SLOT_SIZE);
    memcpy(slot + SSP1_TAG, "SSP1", 4);
    ssp1_le(slot + SSP1_SEQUENCE, sequence, 4);
    slot[SSP1_STATE] = state;
    slot[SSP1_ATTEMPTS] = attempts;
    slot[SSP1_FLAGS] = armed ? 0x01u : 0x00u;
    ssp1_le(slot + SSP1_REMAINING, remaining, 4);
    ssp1_le(slot + SSP1_CRC, ssp1_crc(slot, SSP1_CRC), 4);
    slot[SSP1_COMMIT] = 0xA5u;
}


/* Decode without trusting it: tag, commit marker, CRC and any interesting field. */
static bool ssp1_read(const uint8_t slot[SSP1_SLOT_SIZE], Ssp1Record *out) {
    out->committed = slot[SSP1_COMMIT] == 0xA5u;
    out->sequence = ssp1_le_read(slot + SSP1_SEQUENCE, 4);
    out->state = slot[SSP1_STATE];
    out->attempts = slot[SSP1_ATTEMPTS];
    out->flags = slot[SSP1_FLAGS];
    out->remaining = ssp1_le_read(slot + SSP1_REMAINING, 4);
    return out->committed && memcmp(slot + SSP1_TAG, "SSP1", 4) == 0 &&
           ssp1_le_read(slot + SSP1_CRC, 4) == ssp1_crc(slot, SSP1_CRC);
}
