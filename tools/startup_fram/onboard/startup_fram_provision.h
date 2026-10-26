/*
 * startup_fram_provision.h
 *
 * One-shot provisioning of the 48-byte Startup FRAM image (format tag SSP1).
 * Never called from the normal boot path: the only in-tree caller is
 * startup_fram_provision_boot.c, which is compiled only for the separate
 * provisioning firmware image.
 *
 * The wire constants below are the C spelling of the frozen contract that
 * tools/startup_fram/startup_image.py implements on the host; the golden vector
 * in CONTRACT-VECTOR.json is exercised by both test suites.
 *
 * Device access goes through the existing low-level driver entry points
 * (FRAM_readData / FRAM_writeData / FRAM_writeEnable / FRAM_writeDisable), whose
 * bool return values are all checked here. Nothing in this module writes an
 * address above STARTUP_FRAM_LAST_ADDR.
 */

#ifndef STARTUP_FRAM_PROVISION_H
#define STARTUP_FRAM_PROVISION_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* ---- frozen wire layout: byte addresses on the FRAM device ---- */
#define STARTUP_FRAM_IMAGE_SIZE 48u
#define STARTUP_FRAM_LAST_ADDR (STARTUP_FRAM_IMAGE_SIZE - 1u)
#define STARTUP_FRAM_BOOT_COUNTER_ADDR 0u
#define STARTUP_FRAM_ADCS_MASK_ADDR 4u
#define STARTUP_FRAM_SLOT_A_ADDR 8u
#define STARTUP_FRAM_SLOT_B_ADDR 28u
#define STARTUP_FRAM_SLOT_SIZE 20u
/** Slot bytes 0..18: tag, sequence, state, attempts, flags, remaining_ms, CRC. */
#define STARTUP_FRAM_SLOT_PAYLOAD_SIZE 19u

/* ---- slot field offsets, relative to the slot start ---- */
#define STARTUP_FRAM_SLOT_SEQ_OFFSET 4u
#define STARTUP_FRAM_SLOT_STATE_OFFSET 8u
#define STARTUP_FRAM_SLOT_ATTEMPTS_OFFSET 9u
#define STARTUP_FRAM_SLOT_FLAGS_OFFSET 10u
#define STARTUP_FRAM_SLOT_REMAINING_OFFSET 11u
#define STARTUP_FRAM_SLOT_CRC_OFFSET 15u
#define STARTUP_FRAM_SLOT_MARKER_OFFSET 19u

/* ---- record constants ---- */
#define STARTUP_FRAM_TAG_0 0x53u /* 'S' */
#define STARTUP_FRAM_TAG_1 0x53u /* 'S' */
#define STARTUP_FRAM_TAG_2 0x50u /* 'P' */
#define STARTUP_FRAM_TAG_3 0x31u /* '1' */
#define STARTUP_FRAM_COMMIT_MARKER 0xA5u
#define STARTUP_FRAM_INVALID_MARKER 0x00u
#define STARTUP_FRAM_FLAG_ARMED 0x01u
#define STARTUP_FRAM_FLAG_MASK STARTUP_FRAM_FLAG_ARMED

#define STARTUP_FRAM_SEQ_INITIAL 1u
/* Highest sequence a stored record may carry: the loader accepts it and only the
 * next save reports exhaustion, so a general valid image may hold it. */
#define STARTUP_FRAM_SEQ_MAX 0xFFFFFFFFu
#define STARTUP_FRAM_BOOT_COUNT_MAX 0xFFFFFFFEu
#define STARTUP_FRAM_ADCS_RESERVED_MASK 0xFFE00000u /* bits 21..31 must be zero */

/* MCUState numeric values; durable resume points reuse them. */
#define STARTUP_FRAM_STATE_INITIAL_CHECKS 0u
#define STARTUP_FRAM_STATE_WAIT_POWER_ON 1u
#define STARTUP_FRAM_STATE_WAITING_FOR_UPLINK 2u
#define STARTUP_FRAM_STATE_SAT_TO_GROUND_1 3u
#define STARTUP_FRAM_STATE_MAIN_OPS 6u

#define STARTUP_FRAM_ATTEMPT_LIMIT 7u
#define STARTUP_FRAM_INHIBIT_MS 1800000u
#define STARTUP_FRAM_RETRY_MS 172800000u
#define STARTUP_FRAM_RETRY_MS_SATURATED 604800000u

/** Bytes 0..26: boot counter, ADCS mask and the slot A payload without its marker. */
#define STARTUP_FRAM_PREFIX_SIZE \
    (STARTUP_FRAM_SLOT_A_ADDR + STARTUP_FRAM_SLOT_PAYLOAD_SIZE)
/** Bytes 28..46: the slot B payload without its marker. */
#define STARTUP_FRAM_SLOT_B_PAYLOAD_ADDR STARTUP_FRAM_SLOT_B_ADDR
#define STARTUP_FRAM_SLOT_B_PAYLOAD_SIZE STARTUP_FRAM_SLOT_PAYLOAD_SIZE

typedef enum {
    /** The image was written, committed and read back byte for byte. */
    STARTUP_FRAM_PROVISION_OK = 0,
    /** The supplied image violates the contract; no device write was attempted. */
    STARTUP_FRAM_PROVISION_IMAGE_REJECTED = 1,
    /** A commit marker could not be cleared and verified; nothing is published. */
    STARTUP_FRAM_PROVISION_INVALIDATE_FAILED = 2,
    /** A boot/ADCS/payload write or its read-back failed; nothing is published. */
    STARTUP_FRAM_PROVISION_WRITE_FAILED = 3,
    /** A commit marker write or its read-back failed; payload was already verified. */
    STARTUP_FRAM_PROVISION_COMMIT_FAILED = 4,
    /** The final whole-image read-back did not match the intended bytes. */
    STARTUP_FRAM_PROVISION_VERIFY_FAILED = 5,
} StartupFramProvisionStatus;

/**
 * Check a 48-byte image against the wire contract without touching the device.
 *
 * Rejects: wrong length, saturated boot counter, reserved ADCS mask bits, bad
 * tag/CRC/marker, out-of-range or transient resume states, and the per-state
 * attempts/remaining combinations.
 */
bool startup_fram_image_is_valid(const uint8_t *image, size_t length);

/**
 * Provision the device with `image` (addresses 0..47 only).
 *
 * Order, chosen so a power cut can never publish a partially written record:
 * invalidate and verify both commit markers, write and verify boot counter +
 * ADCS mask + both slot payloads, then publish each commit marker and verify the
 * complete 48-byte image. On any failure the caller must keep the pyro OFF and
 * request a reset; the device may then hold an invalid marker pair and needs
 * re-provisioning with the deployment harness disconnected.
 */
StartupFramProvisionStatus startup_fram_provision_image(const uint8_t *image,
                                                       size_t length);

#endif /* STARTUP_FRAM_PROVISION_H */
