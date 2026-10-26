/*
 * startup_fram_provision.c
 *
 * Intentional factory-image provisioning for the 48-byte Startup FRAM record.
 * See startup_fram_provision.h for the ordering contract.
 *
 * The module is device-independent apart from the four checked low-level driver
 * calls, so the host test suite exercises this exact source against a memory
 * FRAM model that can inject short writes, corrupted bytes, lost responses and
 * hard I/O errors at every stage.
 */

#include "startup_fram_provision.h"

#include <string.h>

#include "MB85RS256B.h"

/* Payload stages, in the order they are written. */
#define STARTUP_FRAM_SLOT_A_MARKER_ADDR \
    (STARTUP_FRAM_SLOT_A_ADDR + STARTUP_FRAM_SLOT_MARKER_OFFSET)
#define STARTUP_FRAM_SLOT_B_MARKER_ADDR \
    (STARTUP_FRAM_SLOT_B_ADDR + STARTUP_FRAM_SLOT_MARKER_OFFSET)

static uint32_t startup_fram_read_le32(const uint8_t *data)
{
    return (uint32_t)data[0] | ((uint32_t)data[1] << 8) | ((uint32_t)data[2] << 16)
           | ((uint32_t)data[3] << 24);
}

/* Reflected 0xEDB88320 CRC32, init and final xor 0xFFFFFFFF: zlib compatible. */
static uint32_t startup_fram_crc32(const uint8_t *data, size_t length)
{
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t index = 0; index < length; index++) {
        crc ^= data[index];
        for (uint32_t bit = 0; bit < 8u; bit++) {
            uint32_t mask = 0u - (crc & 1u);
            crc = (crc >> 1) ^ (0xEDB88320u & mask);
        }
    }
    return crc ^ 0xFFFFFFFFu;
}

static bool startup_fram_tag_matches(const uint8_t *slot)
{
    return slot[0] == STARTUP_FRAM_TAG_0 && slot[1] == STARTUP_FRAM_TAG_1
           && slot[2] == STARTUP_FRAM_TAG_2 && slot[3] == STARTUP_FRAM_TAG_3;
}

static bool startup_fram_retry_ms(uint8_t attempts_used, uint32_t *remaining_ms)
{
    if (attempts_used < 1u || attempts_used > STARTUP_FRAM_ATTEMPT_LIMIT) {
        return false;
    }
    *remaining_ms = (attempts_used < STARTUP_FRAM_ATTEMPT_LIMIT)
                        ? STARTUP_FRAM_RETRY_MS
                        : STARTUP_FRAM_RETRY_MS_SATURATED;
    return true;
}

static bool startup_fram_state_is_persistent(uint8_t state)
{
    return state == STARTUP_FRAM_STATE_INITIAL_CHECKS
           || state == STARTUP_FRAM_STATE_WAIT_POWER_ON
           || state == STARTUP_FRAM_STATE_WAITING_FOR_UPLINK
           || state == STARTUP_FRAM_STATE_SAT_TO_GROUND_1
           || state == STARTUP_FRAM_STATE_MAIN_OPS;
}

/* Field-range rules for one slot: the C counterpart of startup_image.slot_field_problems. */
static bool startup_fram_slot_fields_are_valid(uint32_t sequence, uint8_t state,
                                               uint8_t attempts_used, bool armed,
                                               uint32_t remaining_ms)
{
    /* Sequence 0 is never valid; STARTUP_FRAM_SEQ_MAX is a loadable record (the
     * following save is what reports exhaustion), so no upper-bound check. */
    if (sequence < STARTUP_FRAM_SEQ_INITIAL) {
        return false;
    }
    if (!startup_fram_state_is_persistent(state)) {
        return false;
    }
    if (attempts_used > STARTUP_FRAM_ATTEMPT_LIMIT) {
        return false;
    }

    if (state == STARTUP_FRAM_STATE_INITIAL_CHECKS) {
        if (attempts_used != 0u || remaining_ms > STARTUP_FRAM_INHIBIT_MS) {
            return false;
        }
    } else if (state == STARTUP_FRAM_STATE_WAIT_POWER_ON) {
        if (remaining_ms != 0u) {
            return false;
        }
    } else if (state == STARTUP_FRAM_STATE_WAITING_FOR_UPLINK) {
        uint32_t limit_ms = 0u;
        if (!startup_fram_retry_ms(attempts_used, &limit_ms) || remaining_ms > limit_ms) {
            return false;
        }
    } else if (remaining_ms != 0u) {
        /* SAT_TO_GROUND_1 and MAIN_OPS persist with no wait outstanding. */
        return false;
    }

    /* Every non-initial state requires the armed flag; INITIAL_CHECKS may be armed. */
    if (state != STARTUP_FRAM_STATE_INITIAL_CHECKS && !armed) {
        return false;
    }
    return true;
}

/* Validate one raw 20-byte slot: commit marker, tag, CRC, flags, field ranges. */
static bool startup_fram_slot_is_valid(const uint8_t *slot)
{
    if (slot[STARTUP_FRAM_SLOT_MARKER_OFFSET] != STARTUP_FRAM_COMMIT_MARKER) {
        return false;
    }
    if (!startup_fram_tag_matches(slot)) {
        return false;
    }
    if (startup_fram_read_le32(&slot[STARTUP_FRAM_SLOT_CRC_OFFSET])
        != startup_fram_crc32(slot, STARTUP_FRAM_SLOT_CRC_OFFSET)) {
        return false;
    }
    if ((slot[STARTUP_FRAM_SLOT_FLAGS_OFFSET] & (uint8_t)~STARTUP_FRAM_FLAG_MASK) != 0u) {
        return false;
    }
    return startup_fram_slot_fields_are_valid(
        startup_fram_read_le32(&slot[STARTUP_FRAM_SLOT_SEQ_OFFSET]),
        slot[STARTUP_FRAM_SLOT_STATE_OFFSET],
        slot[STARTUP_FRAM_SLOT_ATTEMPTS_OFFSET],
        (slot[STARTUP_FRAM_SLOT_FLAGS_OFFSET] & STARTUP_FRAM_FLAG_ARMED) != 0u,
        startup_fram_read_le32(&slot[STARTUP_FRAM_SLOT_REMAINING_OFFSET]));
}

bool startup_fram_image_is_valid(const uint8_t *image, size_t length)
{
    if (image == NULL || length != STARTUP_FRAM_IMAGE_SIZE) {
        return false;
    }
    if (startup_fram_read_le32(&image[STARTUP_FRAM_BOOT_COUNTER_ADDR])
        > STARTUP_FRAM_BOOT_COUNT_MAX) {
        /* The firmware increments the counter, so a saturated value is unusable. */
        return false;
    }
    if ((startup_fram_read_le32(&image[STARTUP_FRAM_ADCS_MASK_ADDR])
         & STARTUP_FRAM_ADCS_RESERVED_MASK)
        != 0u) {
        return false;
    }
    const uint8_t *slot_a = &image[STARTUP_FRAM_SLOT_A_ADDR];
    const uint8_t *slot_b = &image[STARTUP_FRAM_SLOT_B_ADDR];
    if (!startup_fram_slot_is_valid(slot_a) || !startup_fram_slot_is_valid(slot_b)) {
        return false;
    }
    /* Two records claiming the same sequence must be identical, otherwise the
     * loader sees an ambiguous pair and treats the device as an error. */
    if (startup_fram_read_le32(&slot_a[STARTUP_FRAM_SLOT_SEQ_OFFSET])
            == startup_fram_read_le32(&slot_b[STARTUP_FRAM_SLOT_SEQ_OFFSET])
        && memcmp(slot_a, slot_b, STARTUP_FRAM_SLOT_SIZE) != 0) {
        return false;
    }
    return true;
}

/* ---- checked device access, bounded to addresses 0..47 ---- */

static bool startup_fram_matches(uint16_t address, const uint8_t *expected, uint16_t length)
{
    if (expected == NULL || length == 0u || address > STARTUP_FRAM_LAST_ADDR
        || length > (uint16_t)(STARTUP_FRAM_IMAGE_SIZE - address)) {
        return false;
    }
    for (uint16_t index = 0; index < length; index++) {
        uint8_t readback = 0u;
        if (!FRAM_readData((uint16_t)(address + index), &readback, 1u)
            || readback != expected[index]) {
            return false;
        }
    }
    return true;
}

static bool startup_fram_write_verified(uint16_t address, const uint8_t *data,
                                        uint16_t length)
{
    if (data == NULL || length == 0u || address > STARTUP_FRAM_LAST_ADDR
        || length > (uint16_t)(STARTUP_FRAM_IMAGE_SIZE - address)) {
        return false;
    }
    if (!FRAM_writeEnable()) {
        return false;
    }
    /* The legacy SPI transmit API takes a mutable buffer but does not modify it. */
    if (!FRAM_writeData(address, (uint8_t *)data, length)) {
        (void)FRAM_writeDisable();
        return false;
    }
    if (!FRAM_writeDisable()) {
        return false; /* writable state unknown: treat the update as uncertain */
    }
    return startup_fram_matches(address, data, length);
}

StartupFramProvisionStatus startup_fram_provision_image(const uint8_t *image, size_t length)
{
    const uint8_t invalid = STARTUP_FRAM_INVALID_MARKER;
    const uint8_t commit = STARTUP_FRAM_COMMIT_MARKER;
    uint8_t intended[STARTUP_FRAM_IMAGE_SIZE];

    if (!startup_fram_image_is_valid(image, length)) {
        return STARTUP_FRAM_PROVISION_IMAGE_REJECTED;
    }
    /* Validate before any device access, then work from a private copy: the
     * legacy driver takes a mutable buffer and must not alias caller memory. */
    for (uint16_t index = 0; index < STARTUP_FRAM_IMAGE_SIZE; index++) {
        intended[index] = image[index];
    }

    /* Stage 1: both commit markers invalid and verified before any payload
     * publish, so an interrupted image can never look committed. */
    if (!startup_fram_write_verified((uint16_t)STARTUP_FRAM_SLOT_A_MARKER_ADDR, &invalid, 1u)
        || !startup_fram_write_verified((uint16_t)STARTUP_FRAM_SLOT_B_MARKER_ADDR, &invalid,
                                       1u)) {
        return STARTUP_FRAM_PROVISION_INVALIDATE_FAILED;
    }

    /* Stage 2: boot counter, ADCS mask and both slot payloads, marker bytes
     * excluded, then read back before anything is published. */
    if (!startup_fram_write_verified((uint16_t)STARTUP_FRAM_BOOT_COUNTER_ADDR, intended,
                                     (uint16_t)STARTUP_FRAM_PREFIX_SIZE)
        || !startup_fram_write_verified((uint16_t)STARTUP_FRAM_SLOT_B_PAYLOAD_ADDR,
                                        &intended[STARTUP_FRAM_SLOT_B_PAYLOAD_ADDR],
                                        (uint16_t)STARTUP_FRAM_SLOT_B_PAYLOAD_SIZE)) {
        return STARTUP_FRAM_PROVISION_WRITE_FAILED;
    }
    if (!startup_fram_matches((uint16_t)STARTUP_FRAM_BOOT_COUNTER_ADDR, intended,
                              (uint16_t)STARTUP_FRAM_PREFIX_SIZE)
        || !startup_fram_matches((uint16_t)STARTUP_FRAM_SLOT_B_PAYLOAD_ADDR,
                                 &intended[STARTUP_FRAM_SLOT_B_PAYLOAD_ADDR],
                                 (uint16_t)STARTUP_FRAM_SLOT_B_PAYLOAD_SIZE)) {
        return STARTUP_FRAM_PROVISION_VERIFY_FAILED;
    }

    /* Stage 3: publish each record by writing its commit marker last. */
    if (!startup_fram_write_verified((uint16_t)STARTUP_FRAM_SLOT_A_MARKER_ADDR, &commit, 1u)
        || !startup_fram_write_verified((uint16_t)STARTUP_FRAM_SLOT_B_MARKER_ADDR, &commit,
                                       1u)) {
        return STARTUP_FRAM_PROVISION_COMMIT_FAILED;
    }

    /* Stage 4: the whole intended 0..47 image, byte for byte. */
    if (!startup_fram_matches((uint16_t)STARTUP_FRAM_BOOT_COUNTER_ADDR, intended,
                              (uint16_t)STARTUP_FRAM_IMAGE_SIZE)) {
        return STARTUP_FRAM_PROVISION_VERIFY_FAILED;
    }
    return STARTUP_FRAM_PROVISION_OK;
}
