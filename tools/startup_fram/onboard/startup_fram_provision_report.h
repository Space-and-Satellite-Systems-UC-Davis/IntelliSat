/*
 * startup_fram_provision_report.h
 *
 * Bounded RAM status block that the standalone provisioning image leaves behind
 * for a human with a debugger. It is a fixed-size struct in the `.noinit` linker
 * section, so it survives the NVIC_SystemReset() failure path (it is outside
 * .bss, which the startup code zeroes on every run).
 *
 * It records what the writer decided, what the device actually returned through a
 * fresh FRAM_readData(0, ..., 48) read, and how that read-back differs from the
 * intended image. It is never used by the normal firmware, and its bytes are
 * evidence from the target's own SPI path - not a substitute for an external
 * programmer read-back.
 */

#ifndef STARTUP_FRAM_PROVISION_REPORT_H
#define STARTUP_FRAM_PROVISION_REPORT_H

#include <stdint.h>

#include "startup_fram_provision.h"

/* 'SPRV' - tells a debugger that the block below belongs to this tool. */
#define STARTUP_FRAM_REPORT_MAGIC 0x53505256u
#define STARTUP_FRAM_REPORT_VERSION 1u
#define STARTUP_FRAM_REPORT_ADDR_NONE 0xFFFFFFFFu

typedef struct {
    uint32_t magic;              /* STARTUP_FRAM_REPORT_MAGIC once the run started */
    uint32_t version;            /* STARTUP_FRAM_REPORT_VERSION */
    uint32_t status;             /* StartupFramProvisionStatus of the writer */
    uint32_t image_valid;        /* 1 if the generated image passed validation */
    uint32_t device_read_ok;     /* 1 if FRAM_readData(0, readback, 48) succeeded */
    uint32_t readback_matches;   /* 1 if readback[] equals intended[] byte for byte */
    uint32_t mismatch_count;     /* number of differing bytes in 0..47 */
    uint32_t first_mismatch_addr;/* lowest differing address, or ADDR_NONE */
    uint32_t readback_boot_counter; /* uint32 LE decoded from readback[0..3] */
    uint32_t readback_adcs_mask;    /* uint32 LE decoded from readback[4..7] */
    uint32_t readback_slot_a_marker;/* readback byte 27 */
    uint32_t readback_slot_b_marker;/* readback byte 47 */
    uint8_t intended[STARTUP_FRAM_IMAGE_SIZE]; /* the image the build baked in */
    uint8_t readback[STARTUP_FRAM_IMAGE_SIZE]; /* what the device returned */
} StartupFramProvisionReport;

/**
 * The single report instance. Written only by startup_fram_provision_boot();
 * read by a debugger after the run.
 */
extern volatile StartupFramProvisionReport startup_fram_provision_report;

#endif /* STARTUP_FRAM_PROVISION_REPORT_H */
