#ifndef INTELLISAT_FRAM_LAYOUT_H
#define INTELLISAT_FRAM_LAYOUT_H

/* Preserve origin/main fd4c2bb's boot/ADCS layout. All addresses are bytes. */
#define FRAM_BOOT_COUNTER_ADDR 0U
#define FRAM_ADCS_VARIABLE_ADDR 4U
#define FRAM_MAX_BYTES 0x8000U

/*
 * Startup persistence: two 20-byte SSP1 records, slot A then slot B, at fixed
 * byte addresses 8..47. Nothing else may be written there; Startup and the
 * provisioning writer both stop at byte 47.
 */
#define FRAM_STARTUP_ADDR 8U
#define FRAM_STARTUP_SLOT_SIZE 20U
#define FRAM_STARTUP_SLOT_A_ADDR FRAM_STARTUP_ADDR
#define FRAM_STARTUP_SLOT_B_ADDR (FRAM_STARTUP_ADDR + FRAM_STARTUP_SLOT_SIZE)
#define FRAM_STARTUP_IMAGE_SIZE (2U * FRAM_STARTUP_SLOT_SIZE)

#endif
