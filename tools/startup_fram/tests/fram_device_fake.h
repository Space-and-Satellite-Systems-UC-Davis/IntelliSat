/*
 * fram_device_fake.h
 *
 * Memory model of the MB85RS256B device behind the real low-level driver entry
 * points. It implements the exact prototypes from
 * Src/peripherals/FRAM/MB85RS256B.h, so the compiler checks that
 * startup_fram_provision.c is calling the real interface, and it records every
 * byte range written so tests can assert the 0..47 restriction and the
 * invalidate-then-publish ordering.
 *
 * Faults are injected per write operation index (0-based, in call order) to
 * model a power cut or bus error at any stage.
 */

#ifndef FRAM_DEVICE_FAKE_H
#define FRAM_DEVICE_FAKE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Oldest logged writes kept; the writer performs at most six. */
#define FAKE_FRAM_MAX_WRITE_OPS 32u
/* Device model size: large enough to observe any write that leaves 0..47. */
#define FAKE_FRAM_SIZE 256u

typedef enum {
    FAKE_FAULT_NONE = 0,
    /** Accept only `detail` bytes of the request, report success (power cut mid-stream). */
    FAKE_FAULT_SHORT_WRITE,
    /** Apply the whole request but store one bit wrong in byte `detail`. */
    FAKE_FAULT_CORRUPT_BYTE,
    /** Apply the whole request, then lose the response (bus error after the write). */
    FAKE_FAULT_LOST_RESPONSE,
    /** Apply nothing and report failure. */
    FAKE_FAULT_HARD_ERROR,
    /** Apply the whole request, then fail `detail` (default 1) subsequent reads. */
    FAKE_FAULT_READ_ERROR,
    /** Apply the whole request, then fail the following write-disable. */
    FAKE_FAULT_WRDI_ERROR,
} FakeFramFault;

/** Blank the model (0xFF), clear faults, logging and counters. */
void fake_fram_reset(void);
/** Fill the whole model with `value`. */
void fake_fram_fill(uint8_t value);
/** Copy `length` bytes into the model at `address`. */
void fake_fram_seed(uint16_t address, const uint8_t *data, uint16_t length);
const uint8_t *fake_fram_memory(void);
uint8_t fake_fram_byte(uint16_t address);

/** Arm `fault` on write operation `op_index`. */
void fake_fram_fail_write(unsigned op_index, FakeFramFault fault, unsigned detail);
/** Make write-enable fail on write operation `op_index`. */
void fake_fram_fail_write_enable(unsigned op_index);

unsigned fake_fram_write_ops(void);
uint16_t fake_fram_write_address(unsigned op_index);
uint16_t fake_fram_write_length(unsigned op_index);
unsigned fake_fram_read_ops(void);
/** True if any request touched an address outside FAKE_FRAM_SIZE. */
bool fake_fram_out_of_range(void);

#endif /* FRAM_DEVICE_FAKE_H */
