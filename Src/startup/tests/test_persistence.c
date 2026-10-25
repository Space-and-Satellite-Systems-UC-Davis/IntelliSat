#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "framework.h"
#include "startup_persistence.h"
#include "peripherals/FRAM/MB85RS256B.h"

/* Count evaluated persistence assertions separately from scenario functions/byte cuts. */
static unsigned long s_foundation_assertions;
static unsigned s_foundation_scenarios;
#undef assert
#define assert(condition) do { \
    s_foundation_assertions++; \
    if (!(condition)) { \
        fprintf(stderr, "Assertion failed: %s (%s:%d)\n", #condition, __FILE__, __LINE__); \
        abort(); \
    } \
} while (0)
#define RUN_FOUNDATION(test) do { \
    s_foundation_scenarios++; \
    printf("  %s\n", #test); \
    test(); \
} while (0)

/* Host memory FRAM model */
static uint8_t s_fram[32768];
static uint8_t s_sentinel[8] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x11, 0x22, 0x33, 0x44 };

static bool s_wren_enabled = false;
static bool s_inject_wren_fail = false;
static bool s_inject_wrdi_fail = false;
static bool s_inject_write_fail = false;
static bool s_inject_read_fail = false;
static int s_cut_after_writes = -1;
static int s_write_count = 0;
static int s_read_count, s_wren_count, s_wrdi_count;
static int s_fail_read_call = -1, s_fail_write_call = -1;
static int s_fail_wren_call = -1, s_fail_wrdi_call = -1;
static int s_noop_write_call = -1, s_corrupt_write_call = -1;
static int s_byte_budget = -1;
static unsigned s_written_bytes;
static bool s_power_lost;
/* Adversarial detector test only: one misleading successful full-snapshot read. */
static int s_substitute_read_call = -1;
static unsigned s_substituted_reads;
static uint8_t s_substitute_read_bytes[320];

/* MB85RS256B real declarations implementation for host testing */
bool FRAM_readData(uint16_t address, uint8_t *buffer, uint16_t size) {
    s_read_count++;
    if (s_power_lost || s_inject_read_fail || s_read_count == s_fail_read_call) return false;
    if ((uint32_t)address + size > sizeof(s_fram)) return false;
    if (s_read_count == s_substitute_read_call) {
        assert(address == 8 && size == sizeof(s_substitute_read_bytes));
        memcpy(buffer, s_substitute_read_bytes, size);
        s_substituted_reads++;
    } else {
        memcpy(buffer, &s_fram[address], size);
    }
    return true;
}

bool FRAM_writeData(uint16_t address, uint8_t *data, uint16_t size) {
    s_write_count++;
    if (!s_wren_enabled || s_power_lost || s_inject_write_fail ||
        s_write_count == s_fail_write_call) return false;
    if ((uint32_t)address + size > sizeof(s_fram)) return false;
    if (s_cut_after_writes >= 0 && s_write_count > s_cut_after_writes) return false;
    if (s_write_count == s_noop_write_call) return true;
    for (uint16_t i = 0; i < size; i++) {
        if (s_byte_budget == 0) { s_power_lost = true; return false; }
        s_fram[address + i] = data[i];
        s_written_bytes++;
        if (s_byte_budget > 0 && --s_byte_budget == 0) {
            s_power_lost = true;
            return false; /* The written prefix survives the next public st_load. */
        }
    }
    if (s_write_count == s_corrupt_write_call) s_fram[address] ^= 0x80;
    return true;
}

bool FRAM_writeEnable(void) {
    s_wren_count++;
    if (s_power_lost || s_inject_wren_fail || s_wren_count == s_fail_wren_call) return false;
    s_wren_enabled = true;
    return true;
}

bool FRAM_writeDisable(void) {
    s_wrdi_count++;
    if (s_power_lost || s_inject_wrdi_fail || s_wrdi_count == s_fail_wrdi_call) return false;
    s_wren_enabled = false;
    return true;
}

/* Reset only volatile device/fault state; power loss never clears persistent media. */
static void fram_faults_reset(void) {
    s_write_count = s_read_count = s_wren_count = s_wrdi_count = 0;
    s_written_bytes = 0;
    s_substitute_read_call = -1;
    s_substituted_reads = 0;
    s_fail_read_call = s_fail_write_call = s_fail_wren_call = s_fail_wrdi_call = -1;
    s_noop_write_call = s_corrupt_write_call = s_byte_budget = s_cut_after_writes = -1;
    s_power_lost = false;
    s_wren_enabled = false;
    s_inject_wren_fail = false;
    s_inject_wrdi_fail = false;
    s_inject_write_fail = false;
    s_inject_read_fail = false;
}

static void fram_model_reset(void) {
    memset(s_fram, 0, sizeof(s_fram));
    memcpy(s_fram, s_sentinel, 8);
    fram_faults_reset();
}

static void check_sentinels_unmodified(void) {
    assert(memcmp(&s_fram[0], s_sentinel, 8) == 0);
}

/* Independent golden fixture seeder */
static void seed_golden_initial_fixture(void) {
    fram_model_reset();
    uint8_t golden_crit[96];
    memset(golden_crit, 0, 96);
    golden_crit[0] = 'S'; golden_crit[1] = 'T'; golden_crit[2] = 'C'; golden_crit[3] = '3';
    golden_crit[4] = 0x03; golden_crit[5] = 0x00; /* Version 3 */
    golden_crit[6] = 0x60; golden_crit[7] = 0x00; /* Length 96 */
    golden_crit[8] = 0x01; /* Arming identity = 1 */
    golden_crit[74] = 0x01; /* Authorization = 1 (Armed) */
    /* CRC32: 0xf87ebba0 -> little-endian bytes: a0 bb 7e f8 */
    golden_crit[92] = 0xa0; golden_crit[93] = 0xbb; golden_crit[94] = 0x7e; golden_crit[95] = 0xf8;

    /* Write Critical A @ 8 and Critical B @ 104 */
    memcpy(&s_fram[8], golden_crit, 96);
    memcpy(&s_fram[104], golden_crit, 96);
    /* Progress A @ 200 and Progress B @ 264 are 64 zero bytes */
    memset(&s_fram[200], 0, 64);
    memset(&s_fram[264], 0, 64);
}

static void test_load_golden_fixture(void) {
    seed_golden_initial_fixture();
    check_sentinels_unmodified();

    StSnapshot snap;
    memset(&snap, 0, sizeof(snap));
    StResult res = st_load(&snap);
    assert(res == ST_OK);
    assert(snap.critical_mirrored == true);
    assert(snap.critical_slot == 0);
    assert(snap.has_progress == false);
    assert(snap.control.phase == ST_PHASE_PREPARED);
    assert(snap.control.arming == 1);
    assert(snap.control.sequence == 0);
    assert(snap.control.authorization == 1);
    assert(snap.control.reservation == 0);
    assert(snap.control.wait == 0);
    assert(snap.control.duration_ms == 0);
    assert(snap.control.elapsed_ms == 0);
    assert(snap.control.flags == 0);
    assert(snap.control.fast_count == 0);
    check_sentinels_unmodified();
}

static void test_unmirrored_recovery_and_repair(void) {
    /* Case 1: Critical A valid, Critical B corrupted */
    seed_golden_initial_fixture();
    memset(&s_fram[104], 0xFF, 96); /* Corrupt replica B */

    StSnapshot snap;
    StResult res = st_load(&snap);
    assert(res == ST_RECOVERABLE);
    assert(snap.critical_mirrored == false);
    assert(snap.critical_slot == 0);
    assert(snap.control.sequence == 0);
    assert(snap.control.phase == ST_PHASE_PREPARED);
    check_sentinels_unmodified();

    /* Repair mirrors A to B */
    StSnapshot repaired;
    res = st_repair(&snap, &repaired);
    assert(res == ST_OK);
    assert(repaired.critical_mirrored == true);
    assert(memcmp(&s_fram[8], &s_fram[104], 96) == 0);
    check_sentinels_unmodified();

    /* Reload confirms clean OK */
    StSnapshot reloaded;
    res = st_load(&reloaded);
    assert(res == ST_OK);
    assert(reloaded.critical_mirrored == true);

    /* Case 2: Legal predecessor/successor pair (PREPARED @ 8, INHIBIT @ 104) */
    seed_golden_initial_fixture();
    StControl inhibit_ctrl = {
        .arming = 1,
        .sequence = 1,
        .reservation = 0,
        .wait = 1,
        .duration_ms = 1800000,
        .elapsed_ms = 0,
        .reference_ms = 500000,
        .source = 1,
        .flags = 0,
        .phase = ST_PHASE_INHIBIT,
        .fast_count = 0,
        .authorization = 1,
        .reference_valid = 1
    };
    /* Encode into slot B @ 104 */
    uint8_t b_rec[96];
    memset(b_rec, 0, 96);
    b_rec[0] = 'S'; b_rec[1] = 'T'; b_rec[2] = 'C'; b_rec[3] = '3';
    b_rec[4] = 0x03; b_rec[5] = 0x00;
    b_rec[6] = 0x60; b_rec[7] = 0x00;
    b_rec[8] = 0x01; /* arming 1 */
    b_rec[16] = 0x01; /* sequence 1 */
    b_rec[32] = 0x01; /* wait 1 */
    /* duration: 1800000 -> 0x001B7740 */
    b_rec[40] = 0x40; b_rec[41] = 0x77; b_rec[42] = 0x1B;
    /* reference_ms: 500000 -> 0x0007A120 */
    b_rec[56] = 0x20; b_rec[57] = 0xA1; b_rec[58] = 0x07;
    b_rec[64] = 0x01; /* source 1 */
    b_rec[72] = ST_PHASE_INHIBIT;
    b_rec[74] = 0x01; /* authorization 1 */
    b_rec[75] = 0x01; /* reference valid 1 */
    /* CRC over bytes 4..91 */
    uint32_t b_crc = 0xFFFFFFFFU;
    for (size_t i = 4; i < 92; i++) {
        b_crc ^= (uint32_t)b_rec[i];
        for (int b = 0; b < 8; b++) {
            b_crc = (b_crc & 1U) ? ((b_crc >> 1) ^ 0xEDB88320U) : (b_crc >> 1);
        }
    }
    b_crc ^= 0xFFFFFFFFU;
    b_rec[92] = (uint8_t)(b_crc & 0xFFU);
    b_rec[93] = (uint8_t)((b_crc >> 8) & 0xFFU);
    b_rec[94] = (uint8_t)((b_crc >> 16) & 0xFFU);
    b_rec[95] = (uint8_t)((b_crc >> 24) & 0xFFU);
    memcpy(&s_fram[104], b_rec, 96);

    res = st_load(&snap);
    assert(res == ST_RECOVERABLE);
    assert(snap.critical_mirrored == false);
    assert(snap.critical_slot == 1); /* successor in slot B selected */
    assert(snap.control.sequence == 1);
    assert(snap.control.phase == ST_PHASE_INHIBIT);

    /* Repair mirrors B to A */
    res = st_repair(&snap, &repaired);
    assert(res == ST_OK);
    assert(repaired.critical_mirrored == true);
    assert(repaired.control.sequence == 1);
    assert(repaired.control.phase == ST_PHASE_INHIBIT);
    assert(memcmp(&s_fram[8], &s_fram[104], 96) == 0);

    /* Repair already mirrored: no write, returns ST_OK */
    res = st_repair(&repaired, &snap);
    assert(res == ST_OK);
    assert(snap.critical_mirrored == true);
    (void)inhibit_ctrl;
}

static void test_invalid_fixture_handling(void) {
    StSnapshot snap;

    /* 1. All blank (zero bytes) */
    fram_model_reset();
    assert(st_load(&snap) == ST_INVALID);

    /* 2. Corrupt CRC on both copies */
    seed_golden_initial_fixture();
    s_fram[8 + 92] ^= 0x01;
    s_fram[104 + 92] ^= 0x01;
    assert(st_load(&snap) == ST_INVALID);

    /* 3. Bad commit string on both copies */
    seed_golden_initial_fixture();
    s_fram[8] = 'X';
    s_fram[104] = 'X';
    assert(st_load(&snap) == ST_INVALID);

    /* 4. Bad version on both */
    seed_golden_initial_fixture();
    s_fram[8 + 4] = 4;
    s_fram[104 + 4] = 4;
    assert(st_load(&snap) == ST_INVALID);

    /* 5. Nonzero reserved bytes */
    seed_golden_initial_fixture();
    s_fram[8 + 76] = 1; /* reserved byte */
    s_fram[104 + 76] = 1;
    assert(st_load(&snap) == ST_INVALID);

    /* 6. Equal sequence, different content (tied disagreement) */
    seed_golden_initial_fixture();
    /* Both are valid critical PREPARED records, but with different arming identity */
    uint8_t alt_crit[96];
    memcpy(alt_crit, &s_fram[8], 96);
    alt_crit[8] = 2; /* arming 2 instead of 1 */
    uint32_t alt_crc = 0xFFFFFFFFU;
    for (size_t i = 4; i < 92; i++) {
        alt_crc ^= (uint32_t)alt_crit[i];
        for (int b = 0; b < 8; b++) {
            alt_crc = (alt_crc & 1U) ? ((alt_crc >> 1) ^ 0xEDB88320U) : (alt_crc >> 1);
        }
    }
    alt_crc ^= 0xFFFFFFFFU;
    alt_crit[92] = (uint8_t)(alt_crc & 0xFFU);
    alt_crit[93] = (uint8_t)((alt_crc >> 8) & 0xFFU);
    alt_crit[94] = (uint8_t)((alt_crc >> 16) & 0xFFU);
    alt_crit[95] = (uint8_t)((alt_crc >> 24) & 0xFFU);
    memcpy(&s_fram[104], alt_crit, 96);
    StResult r6 = st_load(&snap);
    assert(r6 == ST_INVALID);

    /* 7. Sequence gap > 1 (e.g. seq 0 @ 8 and seq 2 @ 104) */
    seed_golden_initial_fixture();
    uint8_t seq2_crit[96];
    memset(seq2_crit, 0, 96);
    seq2_crit[0] = 'S'; seq2_crit[1] = 'T'; seq2_crit[2] = 'C'; seq2_crit[3] = '3';
    seq2_crit[4] = 3; seq2_crit[6] = 96;
    seq2_crit[8] = 1;  /* arming 1 */
    seq2_crit[16] = 2; /* sequence 2 */
    seq2_crit[68] = 1; /* flags: INHIBIT_DONE */
    seq2_crit[72] = ST_PHASE_ELIGIBLE;
    seq2_crit[74] = 1; /* authorization 1 */
    uint32_t seq2_crc = 0xFFFFFFFFU;
    for (size_t i = 4; i < 92; i++) {
        seq2_crc ^= (uint32_t)seq2_crit[i];
        for (int b = 0; b < 8; b++) {
            seq2_crc = (seq2_crc & 1U) ? ((seq2_crc >> 1) ^ 0xEDB88320U) : (seq2_crc >> 1);
        }
    }
    seq2_crc ^= 0xFFFFFFFFU;
    seq2_crit[92] = (uint8_t)(seq2_crc & 0xFFU);
    seq2_crit[93] = (uint8_t)((seq2_crc >> 8) & 0xFFU);
    seq2_crit[94] = (uint8_t)((seq2_crc >> 16) & 0xFFU);
    seq2_crit[95] = (uint8_t)((seq2_crc >> 24) & 0xFFU);
    memcpy(&s_fram[104], seq2_crit, 96);
    assert(st_load(&snap) == ST_INVALID);

    /* 8. PREPARED with non-clear wait (e.g. duration != 0) */
    seed_golden_initial_fixture();
    s_fram[8 + 40] = 0x50; /* duration byte */
    uint32_t bad_crc = 0xFFFFFFFFU;
    for (size_t i = 4; i < 92; i++) {
        bad_crc ^= (uint32_t)s_fram[8 + i];
        for (int b = 0; b < 8; b++) {
            bad_crc = (bad_crc & 1U) ? ((bad_crc >> 1) ^ 0xEDB88320U) : (bad_crc >> 1);
        }
    }
    bad_crc ^= 0xFFFFFFFFU;
    s_fram[8 + 92] = (uint8_t)(bad_crc & 0xFFU);
    s_fram[8 + 93] = (uint8_t)((bad_crc >> 8) & 0xFFU);
    s_fram[8 + 94] = (uint8_t)((bad_crc >> 16) & 0xFFU);
    s_fram[8 + 95] = (uint8_t)((bad_crc >> 24) & 0xFFU);
    memcpy(&s_fram[104], &s_fram[8], 96);
    assert(st_load(&snap) == ST_INVALID);

    check_sentinels_unmodified();
}

static void test_io_and_driver_fault_injection(void) {
    seed_golden_initial_fixture();
    StSnapshot snap;
    StClock clock = { .epoch_ms = 100000, .source = 1, .revision = 0, .valid = true };

    /* 1. Injected read failure on load */
    s_inject_read_fail = true;
    assert(st_load(&snap) == ST_IO);
    s_inject_read_fail = false;
    assert(st_load(&snap) == ST_OK);

    /* 2. Injected write failure during repair */
    memset(&s_fram[104], 0xFF, 96); /* Make unmirrored */
    assert(st_load(&snap) == ST_RECOVERABLE);
    s_inject_write_fail = true;
    StSnapshot repaired;
    assert(st_repair(&snap, &repaired) == ST_IO);
    s_inject_write_fail = false;
    assert(st_repair(&snap, &repaired) == ST_OK);

    /* 3. Injected WREN failure during start_inhibit */
    seed_golden_initial_fixture();
    assert(st_load(&snap) == ST_OK);
    s_inject_wren_fail = true;
    StSnapshot started;
    assert(st_start_inhibit(&snap, &clock, &started) == ST_IO);
    s_inject_wren_fail = false;
    assert(st_start_inhibit(&snap, &clock, &started) == ST_OK);
    assert(started.control.phase == ST_PHASE_INHIBIT);

    /* 4. Injected WRDI cleanup failure during checkpoint */
    StWaitCursor cursor;
    assert(st_wait_open(&started, &clock, ST_NO_OLD_TIME_CREDIT, &cursor) == ST_OK);
    clock.epoch_ms += 60000;
    s_inject_wrdi_fail = true;
    StSnapshot checkpointed;
    assert(st_checkpoint(&started, &cursor, &clock, &checkpointed) == ST_IO);
    s_inject_wrdi_fail = false;

    check_sentinels_unmodified();
}

static void helper_encode_critical(const StControl *c, uint8_t *raw) {
    memset(raw, 0, 96);
    raw[0] = 'S'; raw[1] = 'T'; raw[2] = 'C'; raw[3] = '3';
    raw[4] = 3; raw[6] = 96;
    for (int i = 0; i < 8; i++) {
        raw[8 + i] = (uint8_t)((c->arming >> (8 * i)) & 0xFFU);
        raw[16 + i] = (uint8_t)((c->sequence >> (8 * i)) & 0xFFU);
        raw[24 + i] = (uint8_t)((c->reservation >> (8 * i)) & 0xFFU);
        raw[32 + i] = (uint8_t)((c->wait >> (8 * i)) & 0xFFU);
        raw[40 + i] = (uint8_t)((c->duration_ms >> (8 * i)) & 0xFFU);
        raw[48 + i] = (uint8_t)((c->elapsed_ms >> (8 * i)) & 0xFFU);
        raw[56 + i] = (uint8_t)((c->reference_ms >> (8 * i)) & 0xFFU);
    }
    for (int i = 0; i < 4; i++) {
        raw[64 + i] = (uint8_t)((c->source >> (8 * i)) & 0xFFU);
        raw[68 + i] = (uint8_t)((c->flags >> (8 * i)) & 0xFFU);
    }
    raw[72] = c->phase;
    raw[73] = c->fast_count;
    raw[74] = c->authorization;
    raw[75] = c->reference_valid;

    uint32_t crc = 0xFFFFFFFFU;
    for (size_t i = 4; i < 92; i++) {
        crc ^= (uint32_t)raw[i];
        for (int b = 0; b < 8; b++) {
            crc = (crc & 1U) ? ((crc >> 1) ^ 0xEDB88320U) : (crc >> 1);
        }
    }
    crc ^= 0xFFFFFFFFU;
    raw[92] = (uint8_t)(crc & 0xFFU);
    raw[93] = (uint8_t)((crc >> 8) & 0xFFU);
    raw[94] = (uint8_t)((crc >> 16) & 0xFFU);
    raw[95] = (uint8_t)((crc >> 24) & 0xFFU);
}

static void test_all_seven_phases_and_legal_pairs(void) {
    StSnapshot snap;
    uint8_t rawA[96], rawB[96];

    StControl phases[7] = {
        /* 0: PREPARED */
        { .arming = 1, .sequence = 0, .reservation = 0, .wait = 0, .duration_ms = 0, .elapsed_ms = 0, .reference_ms = 0, .source = 0, .flags = 0, .phase = ST_PHASE_PREPARED, .fast_count = 0, .authorization = 1, .reference_valid = 0 },
        /* 1: INHIBIT */
        { .arming = 1, .sequence = 1, .reservation = 0, .wait = 1, .duration_ms = 1800000, .elapsed_ms = 0, .reference_ms = 1000, .source = 1, .flags = 0, .phase = ST_PHASE_INHIBIT, .fast_count = 0, .authorization = 1, .reference_valid = 1 },
        /* 2: ELIGIBLE */
        { .arming = 1, .sequence = 2, .reservation = 0, .wait = 0, .duration_ms = 0, .elapsed_ms = 0, .reference_ms = 0, .source = 0, .flags = 1, .phase = ST_PHASE_ELIGIBLE, .fast_count = 0, .authorization = 1, .reference_valid = 0 },
        /* 3: RESERVED */
        { .arming = 1, .sequence = 3, .reservation = 3, .wait = 0, .duration_ms = 0, .elapsed_ms = 0, .reference_ms = 0, .source = 0, .flags = 1, .phase = ST_PHASE_RESERVED, .fast_count = 1, .authorization = 1, .reference_valid = 0 },
        /* 4: RETRY_WAIT */
        { .arming = 1, .sequence = 4, .reservation = 3, .wait = 4, .duration_ms = 172800000, .elapsed_ms = 0, .reference_ms = 2000, .source = 1, .flags = 1, .phase = ST_PHASE_RETRY_WAIT, .fast_count = 1, .authorization = 1, .reference_valid = 1 },
        /* 5: RETIRED */
        { .arming = 1, .sequence = 5, .reservation = 3, .wait = 0, .duration_ms = 0, .elapsed_ms = 0, .reference_ms = 0, .source = 0, .flags = 1, .phase = ST_PHASE_RETIRED, .fast_count = 1, .authorization = 1, .reference_valid = 0 },
        /* 6: COMPLETE */
        { .arming = 1, .sequence = 6, .reservation = 3, .wait = 0, .duration_ms = 0, .elapsed_ms = 0, .reference_ms = 0, .source = 0, .flags = 1, .phase = ST_PHASE_COMPLETE, .fast_count = 1, .authorization = 1, .reference_valid = 0 },
    };

    /* Validate each phase individually as mirrored */
    for (int p = 0; p < 7; p++) {
        fram_model_reset();
        helper_encode_critical(&phases[p], rawA);
        memcpy(&s_fram[8], rawA, 96);
        memcpy(&s_fram[104], rawA, 96);
        assert(st_load(&snap) == ST_OK);
        assert(snap.critical_mirrored == true);
        assert(snap.control.phase == phases[p].phase);
        assert(snap.control.sequence == phases[p].sequence);
    }

    /* Validate legal predecessor/successor pairs */
    /* 1. PREPARED(0) -> INHIBIT(1) */
    fram_model_reset();
    helper_encode_critical(&phases[0], rawA);
    helper_encode_critical(&phases[1], rawB);
    memcpy(&s_fram[8], rawA, 96);
    memcpy(&s_fram[104], rawB, 96);
    assert(st_load(&snap) == ST_RECOVERABLE);
    assert(snap.critical_slot == 1); /* successor selected */
    assert(snap.control.phase == ST_PHASE_INHIBIT);

    /* 2. INHIBIT(1) -> ELIGIBLE(2) */
    fram_model_reset();
    helper_encode_critical(&phases[1], rawA);
    helper_encode_critical(&phases[2], rawB);
    memcpy(&s_fram[8], rawA, 96);
    memcpy(&s_fram[104], rawB, 96);
    assert(st_load(&snap) == ST_RECOVERABLE);
    assert(snap.critical_slot == 1);
    assert(snap.control.phase == ST_PHASE_ELIGIBLE);

    /* 3. ELIGIBLE(2) -> RESERVED(3) */
    fram_model_reset();
    helper_encode_critical(&phases[2], rawA);
    helper_encode_critical(&phases[3], rawB);
    memcpy(&s_fram[8], rawA, 96);
    memcpy(&s_fram[104], rawB, 96);
    assert(st_load(&snap) == ST_RECOVERABLE);
    assert(snap.critical_slot == 1);
    assert(snap.control.phase == ST_PHASE_RESERVED);

    /* 4. RESERVED(3) -> RETRY_WAIT(4) */
    fram_model_reset();
    helper_encode_critical(&phases[3], rawA);
    helper_encode_critical(&phases[4], rawB);
    memcpy(&s_fram[8], rawA, 96);
    memcpy(&s_fram[104], rawB, 96);
    assert(st_load(&snap) == ST_RECOVERABLE);
    assert(snap.critical_slot == 1);
    assert(snap.control.phase == ST_PHASE_RETRY_WAIT);

    /* 5. RETRY_WAIT(4) -> RESERVED(5) */
    StControl res_from_retry = phases[3];
    res_from_retry.sequence = 5;
    res_from_retry.reservation = 5;
    res_from_retry.fast_count = 2;
    fram_model_reset();
    helper_encode_critical(&phases[4], rawA);
    helper_encode_critical(&res_from_retry, rawB);
    memcpy(&s_fram[8], rawA, 96);
    memcpy(&s_fram[104], rawB, 96);
    assert(st_load(&snap) == ST_RECOVERABLE);
    assert(snap.critical_slot == 1);
    assert(snap.control.phase == ST_PHASE_RESERVED);

    /* 6. ELIGIBLE(2) -> RETIRED(3) (reservation zero path) */
    StControl ret_from_eligible = { .arming = 1, .sequence = 3, .reservation = 0, .wait = 0, .duration_ms = 0, .elapsed_ms = 0, .reference_ms = 0, .source = 0, .flags = 1, .phase = ST_PHASE_RETIRED, .fast_count = 0, .authorization = 1, .reference_valid = 0 };
    fram_model_reset();
    helper_encode_critical(&phases[2], rawA);
    helper_encode_critical(&ret_from_eligible, rawB);
    memcpy(&s_fram[8], rawA, 96);
    memcpy(&s_fram[104], rawB, 96);
    assert(st_load(&snap) == ST_RECOVERABLE);
    assert(snap.critical_slot == 1);
    assert(snap.control.phase == ST_PHASE_RETIRED);

    /* 7. RETRY_WAIT(4) -> RETIRED(5) */
    fram_model_reset();
    helper_encode_critical(&phases[4], rawA);
    helper_encode_critical(&phases[5], rawB);
    memcpy(&s_fram[8], rawA, 96);
    memcpy(&s_fram[104], rawB, 96);
    assert(st_load(&snap) == ST_RECOVERABLE);
    assert(snap.critical_slot == 1);
    assert(snap.control.phase == ST_PHASE_RETIRED);

    /* 8. RETIRED(5) -> COMPLETE(6) */
    fram_model_reset();
    helper_encode_critical(&phases[5], rawA);
    helper_encode_critical(&phases[6], rawB);
    memcpy(&s_fram[8], rawA, 96);
    memcpy(&s_fram[104], rawB, 96);
    assert(st_load(&snap) == ST_RECOVERABLE);
    assert(snap.critical_slot == 1);
    assert(snap.control.phase == ST_PHASE_COMPLETE);

    /* Incompatible valid pairs are REJECTED as ST_INVALID */
    /* 1. Gap > 1: PREPARED(seq 0) + ELIGIBLE(seq 2) */
    fram_model_reset();
    helper_encode_critical(&phases[0], rawA); /* PREPARED seq 0 */
    helper_encode_critical(&phases[2], rawB); /* ELIGIBLE seq 2 */
    memcpy(&s_fram[8], rawA, 96);
    memcpy(&s_fram[104], rawB, 96);
    assert(st_load(&snap) == ST_INVALID);

    /* 2. Gap > 1: INHIBIT(seq 1) + RESERVED(seq 3) */
    fram_model_reset();
    helper_encode_critical(&phases[1], rawA); /* INHIBIT seq 1 */
    helper_encode_critical(&phases[3], rawB); /* RESERVED seq 3 */
    memcpy(&s_fram[8], rawA, 96);
    memcpy(&s_fram[104], rawB, 96);
    assert(st_load(&snap) == ST_INVALID);

    /* 3. Incompatible adjacent pair: RESERVED(seq 3, res 3) + COMPLETE(seq 4, res 0) */
    StControl complete_res0 = { .arming = 1, .sequence = 4, .reservation = 0, .wait = 0, .duration_ms = 0, .elapsed_ms = 0, .reference_ms = 0, .source = 0, .flags = 1, .phase = ST_PHASE_COMPLETE, .fast_count = 0, .authorization = 1, .reference_valid = 0 };
    fram_model_reset();
    helper_encode_critical(&phases[3], rawA); /* RESERVED seq 3, res 3 */
    helper_encode_critical(&complete_res0, rawB); /* COMPLETE seq 4, res 0 */
    memcpy(&s_fram[8], rawA, 96);
    memcpy(&s_fram[104], rawB, 96);
    assert(st_load(&snap) == ST_INVALID);

    check_sentinels_unmodified();
}

static void test_start_inhibit_lifecycle(void) {
    StSnapshot snap;
    StClock clock = { .epoch_ms = 250000, .source = 1, .revision = 0, .valid = true };

    /* 1. Authorization == 0: ground provisioned inhibited image */
    seed_golden_initial_fixture();
    s_fram[8 + 74] = 0; /* authorization = 0 */
    uint32_t auth0_crc = 0xFFFFFFFFU;
    for (size_t i = 4; i < 92; i++) {
        auth0_crc ^= (uint32_t)s_fram[8 + i];
        for (int b = 0; b < 8; b++) {
            auth0_crc = (auth0_crc & 1U) ? ((auth0_crc >> 1) ^ 0xEDB88320U) : (auth0_crc >> 1);
        }
    }
    auth0_crc ^= 0xFFFFFFFFU;
    s_fram[8 + 92] = (uint8_t)(auth0_crc & 0xFFU);
    s_fram[8 + 93] = (uint8_t)((auth0_crc >> 8) & 0xFFU);
    s_fram[8 + 94] = (uint8_t)((auth0_crc >> 16) & 0xFFU);
    s_fram[8 + 95] = (uint8_t)((auth0_crc >> 24) & 0xFFU);
    memcpy(&s_fram[104], &s_fram[8], 96);

    assert(st_load(&snap) == ST_OK);
    assert(snap.control.authorization == 0);
    assert(snap.critical_mirrored == true);

    /* st_start_inhibit on authorization=0 returns ST_INHIBITED without writes */
    StSnapshot started;
    assert(st_start_inhibit(&snap, &clock, &started) == ST_INHIBITED);
    /* Confirm FRAM remains untouched */
    assert(s_fram[8 + 74] == 0);
    assert(s_fram[104 + 74] == 0);

    /* 2. Normal armed PREPARED -> INHIBIT */
    seed_golden_initial_fixture();
    assert(st_load(&snap) == ST_OK);
    assert(snap.control.authorization == 1);
    assert(snap.control.phase == ST_PHASE_PREPARED);

    /* Invalid clock check */
    StClock bad_clock = clock;
    bad_clock.valid = false;
    assert(st_start_inhibit(&snap, &bad_clock, &started) == ST_CLOCK_INVALID);
    bad_clock.valid = true;
    bad_clock.source = 0; /* invalid source */
    assert(st_start_inhibit(&snap, &bad_clock, &started) == ST_CLOCK_INVALID);

    /* Successful start */
    assert(st_start_inhibit(&snap, &clock, &started) == ST_OK);
    assert(started.critical_mirrored == true);
    assert(started.control.phase == ST_PHASE_INHIBIT);
    assert(started.control.sequence == 1);
    assert(started.control.wait == 1);
    assert(started.control.duration_ms == 1800000);
    assert(started.control.elapsed_ms == 0);
    assert(started.control.reference_ms == 250000);
    assert(started.control.source == 1);
    assert(started.control.reference_valid == 1);
    assert(started.has_progress == false);

    /* Check both critical copies in FRAM are identical */
    assert(memcmp(&s_fram[8], &s_fram[104], 96) == 0);

    /* CAS Conflict: calling st_start_inhibit with old stale snap */
    assert(st_start_inhibit(&snap, &clock, &started) == ST_CONFLICT);

    check_sentinels_unmodified();
}

static void test_wait_open_measure_and_checkpoint(void) {
    seed_golden_initial_fixture();
    StSnapshot snap, started;
    StClock clock = { .epoch_ms = 1000000, .source = 1, .revision = 0, .valid = true };

    assert(st_load(&snap) == ST_OK);
    assert(st_start_inhibit(&snap, &clock, &started) == ST_OK);

    /* 1. First boot wait open (ST_NO_OLD_TIME_CREDIT) */
    StWaitCursor cursor;
    assert(st_wait_open(&started, &clock, ST_NO_OLD_TIME_CREDIT, &cursor) == ST_OK);
    assert(cursor.token != 0);

    /* Measure at t0: 0 ms */
    uint64_t elapsed = 999;
    assert(st_wait_measure(&started, &cursor, &clock, &elapsed) == ST_OK);
    assert(elapsed == 0);

    /* Advance time by 60,000 ms (1 minute) within cursor */
    clock.epoch_ms += 60000;
    assert(st_wait_measure(&started, &cursor, &clock, &elapsed) == ST_OK);
    assert(elapsed == 60000);

    /* 2. Delayed first checkpoint is the entry barrier; ordinary progress follows. */
    StSnapshot barrier, cp1;
    assert(st_checkpoint(&started, &cursor, &clock, &barrier) == ST_OK);
    assert(barrier.progress.elapsed_ms == 0);
    assert(barrier.progress.reference_ms == 1000000);
    assert(barrier.progress.sequence == 1);
    assert(st_checkpoint(&barrier, &cursor, &clock, &cp1) == ST_OK);
    assert(cp1.has_progress == true);
    assert(cp1.progress.sequence == 2);
    assert(cp1.progress.elapsed_ms == 60000);
    assert(cp1.progress.reference_ms == 1060000);
    assert(cp1.progress_slot == 1); /* Slot B @ 264, following entry barrier A. */

    /* Stale CAS on checkpoint: using old 'started' snapshot instead of 'cp1' */
    clock.epoch_ms += 10000;
    StSnapshot dummy;
    assert(st_checkpoint(&started, &cursor, &clock, &dummy) == ST_CONFLICT);

    /* Advance another minute (total 120,000 ms = 2 minutes) */
    clock.epoch_ms += 50000;
    assert(st_wait_measure(&cp1, &cursor, &clock, &elapsed) == ST_OK);
    assert(elapsed == 120000);

    /* Next ordinary checkpoint alternates back to Slot A @ 200. */
    StSnapshot cp2;
    assert(st_checkpoint(&cp1, &cursor, &clock, &cp2) == ST_OK);
    assert(cp2.has_progress == true);
    assert(cp2.progress.sequence == 3);
    assert(cp2.progress.elapsed_ms == 120000);
    assert(cp2.progress_slot == 0); /* Slot A @ 200 */

    /* 3. Warm restart simulation (reboot with proven continuity) */
    /* Fresh recovery entry via st_load invalidates old volatile cursor */
    StSnapshot reboot_snap;
    assert(st_load(&reboot_snap) == ST_OK);
    assert(reboot_snap.has_progress == true);
    assert(reboot_snap.progress.sequence == 3);
    assert(reboot_snap.progress.elapsed_ms == 120000);

    /* Old cursor token must now be rejected */
    assert(st_wait_measure(&cp2, &cursor, &clock, &elapsed) == ST_INVALID);

    /* Open wait with proven boot continuity: 30 seconds after saved reference */
    clock.epoch_ms = 1120000 + 30000; /* saved ref was 1120000 */
    StWaitCursor warm_cursor;
    assert(st_wait_open(&reboot_snap, &clock, ST_BOOT_CONTINUITY_PROVEN, &warm_cursor) == ST_OK);
    assert(st_wait_measure(&reboot_snap, &warm_cursor, &clock, &elapsed) == ST_OK);
    /* 120000 + 30000 = 150000 ms: credited only since coherent saved reference! */
    assert(elapsed == 150000);

    /* 4. Lost clock continuity simulation: 25 verified minutes saved, 5 new minutes required */
    /* Provision 25 verified minutes (1,500,000 ms) into Progress A */
    clock.epoch_ms = 2000000;
    StSnapshot cp25;
    assert(st_checkpoint(&reboot_snap, &warm_cursor, &clock, &cp25) == ST_OK);

    /* Modify progress record to reflect 25 verified minutes */
    StProgress prog25 = cp25.progress;
    prog25.sequence = 10;
    prog25.elapsed_ms = 1500000; /* 25 minutes */
    prog25.reference_ms = 2000000;
    prog25.source = 1;
    prog25.reference_valid = 1;
    uint8_t p25_raw[64];
    memset(p25_raw, 0, 64);
    p25_raw[0] = 'S'; p25_raw[1] = 'T'; p25_raw[2] = 'P'; p25_raw[3] = '3';
    p25_raw[4] = 3; p25_raw[6] = 64;
    for (int i = 0; i < 8; i++) {
        p25_raw[8 + i] = (uint8_t)((prog25.arming >> (8 * i)) & 0xFFU);
        p25_raw[16 + i] = (uint8_t)((prog25.wait >> (8 * i)) & 0xFFU);
        p25_raw[24 + i] = (uint8_t)((prog25.sequence >> (8 * i)) & 0xFFU);
        p25_raw[32 + i] = (uint8_t)((prog25.elapsed_ms >> (8 * i)) & 0xFFU);
        p25_raw[40 + i] = (uint8_t)((prog25.reference_ms >> (8 * i)) & 0xFFU);
    }
    p25_raw[48] = 1; /* source 1 */
    p25_raw[52] = 1; /* ref valid 1 */
    uint32_t pcrc = 0xFFFFFFFFU;
    for (size_t i = 4; i < 60; i++) {
        pcrc ^= (uint32_t)p25_raw[i];
        for (int b = 0; b < 8; b++) {
            pcrc = (pcrc & 1U) ? ((pcrc >> 1) ^ 0xEDB88320U) : (pcrc >> 1);
        }
    }
    pcrc ^= 0xFFFFFFFFU;
    p25_raw[60] = (uint8_t)(pcrc & 0xFFU);
    p25_raw[61] = (uint8_t)((pcrc >> 8) & 0xFFU);
    p25_raw[62] = (uint8_t)((pcrc >> 16) & 0xFFU);
    p25_raw[63] = (uint8_t)((pcrc >> 24) & 0xFFU);
    memcpy(&s_fram[200], p25_raw, 64);
    memset(&s_fram[264], 0, 64); /* clear slot B */

    /* Simulate lost clock reboot: ST_NO_OLD_TIME_CREDIT */
    StSnapshot lost_snap;
    assert(st_load(&lost_snap) == ST_OK);
    assert(lost_snap.has_progress == true);
    assert(lost_snap.progress.elapsed_ms == 1500000);

    /* Clock reset to epoch 5000 (continuity lost) */
    StClock new_anchor_clock = { .epoch_ms = 5000, .source = 1, .revision = 1, .valid = true };
    StWaitCursor lost_cursor;
    assert(st_wait_open(&lost_snap, &new_anchor_clock, ST_NO_OLD_TIME_CREDIT, &lost_cursor) == ST_OK);
    assert(st_wait_measure(&lost_snap, &lost_cursor, &new_anchor_clock, &elapsed) == ST_OK);
    assert(elapsed == 1500000); /* retains verified lower bound */

    /* Advance 4 minutes (240,000 ms): 1,740,000 ms (not due yet) */
    new_anchor_clock.epoch_ms += 240000;
    assert(st_wait_measure(&lost_snap, &lost_cursor, &new_anchor_clock, &elapsed) == ST_OK);
    assert(elapsed == 1740000);

    /* Advance 1 more minute (total 5 new minutes = 300,000 ms): qualifies at 1,800,000 ms */
    new_anchor_clock.epoch_ms += 60000;
    assert(st_wait_measure(&lost_snap, &lost_cursor, &new_anchor_clock, &elapsed) == ST_OK);
    assert(elapsed == 1800000);

    /* 5. Clock discontinuity: backward time rejects and invalidates cursor */
    StClock backward_clock = new_anchor_clock;
    backward_clock.epoch_ms -= 1000;
    assert(st_wait_measure(&lost_snap, &lost_cursor, &backward_clock, &elapsed) == ST_REANCHOR_REQUIRED);
    /* Invalidation check: subsequent calls with lost_cursor fail */
    assert(st_wait_measure(&lost_snap, &lost_cursor, &new_anchor_clock, &elapsed) == ST_INVALID);

    check_sentinels_unmodified();
}

static void test_finish_inhibit_exact_boundary(void) {
    seed_golden_initial_fixture();
    StSnapshot snap, started;
    StClock clock = { .epoch_ms = 500000, .source = 1, .revision = 0, .valid = true };

    assert(st_load(&snap) == ST_OK);
    assert(st_start_inhibit(&snap, &clock, &started) == ST_OK);

    StWaitCursor cursor;
    assert(st_wait_open(&started, &clock, ST_NO_OLD_TIME_CREDIT, &cursor) == ST_OK);

    /* 1. Exact subsecond boundary: 1,799,999 ms is NOT DUE */
    clock.epoch_ms = 500000 + 1799999ULL;
    StSnapshot finished;
    assert(st_finish_inhibit(&started, &cursor, &clock, &finished) == ST_NOT_DUE);

    /* Crucial check: verify NO critical write occurred */
    StSnapshot check_snap;
    assert(st_load(&check_snap) == ST_OK);
    assert(check_snap.control.phase == ST_PHASE_INHIBIT);
    assert(check_snap.control.sequence == 1);

    /* Re-open wait after reload (since public st_load invalidated old volatile cursor) */
    assert(st_wait_open(&check_snap, &clock, ST_BOOT_CONTINUITY_PROVEN, &cursor) == ST_OK);

    /* 2. Advance exactly 1 more ms to 1,800,000 ms: QUALIFIES */
    clock.epoch_ms = 500000 + 1800000ULL;
    assert(st_finish_inhibit(&check_snap, &cursor, &clock, &finished) == ST_OK);

    assert(finished.critical_mirrored == true);
    assert(finished.control.phase == ST_PHASE_ELIGIBLE);
    assert(finished.control.sequence == 2);
    assert(finished.control.flags == ST_FLAG_INHIBIT_DONE);
    assert(finished.control.wait == 0);
    assert(finished.control.duration_ms == 0);
    assert(finished.has_progress == false);

    /* Cursor is invalidated after wait ending transition */
    uint64_t dummy_el;
    assert(st_wait_measure(&finished, &cursor, &clock, &dummy_el) == ST_INVALID);

    /* Reload confirms clean mirrored ELIGIBLE in FRAM */
    StSnapshot reloaded;
    assert(st_load(&reloaded) == ST_OK);
    assert(reloaded.critical_mirrored == true);
    assert(reloaded.control.phase == ST_PHASE_ELIGIBLE);
    assert(reloaded.control.sequence == 2);
    assert(memcmp(&s_fram[8], &s_fram[104], 96) == 0);

    check_sentinels_unmodified();
}

static void test_power_cut_matrix(void) {
    /* Test power cuts at every write stage of critical transition st_start_inhibit */
    for (int cut = 0; cut <= 6; cut++) {
        seed_golden_initial_fixture();
        StSnapshot snap, started;
        assert(st_load(&snap) == ST_OK);

        StClock clock = { .epoch_ms = 300000, .source = 1, .revision = 0, .valid = true };
        s_cut_after_writes = cut;
        s_write_count = 0;

        StResult start_res = st_start_inhibit(&snap, &clock, &started);
        /* If cut < 6, st_start_inhibit should have failed with ST_IO due to power cut */
        if (cut < 6) {
            assert(start_res == ST_IO);
        } else {
            assert(start_res == ST_OK);
        }

        /* Power cut recovery: reset cut simulation and load */
        s_cut_after_writes = -1;
        s_write_count = 0;

        StSnapshot recovered;
        StResult rec_res = st_load(&recovered);
        /* Contract invariant: power cut MUST leave either valid mirrored or recoverable legal pair */
        assert(rec_res == ST_OK || rec_res == ST_RECOVERABLE);

        if (rec_res == ST_RECOVERABLE) {
            StSnapshot repaired;
            assert(st_repair(&recovered, &repaired) == ST_OK);
            assert(repaired.critical_mirrored == true);
        }

        check_sentinels_unmodified();
    }
}

static void test_decoded_authorization_is_not_authority(void) {
    seed_golden_initial_fixture();
    StControl inhibited = {.arming = 1};
    uint8_t record[96];
    helper_encode_critical(&inhibited, record);
    memcpy(s_fram + 8, record, 96);
    memcpy(s_fram + 104, record, 96);
    StSnapshot snapshot, out;
    StClock clock = {.epoch_ms = 1000000, .source = 1, .valid = true};
    assert(st_load(&snapshot) == ST_OK);
    snapshot.control.authorization = 1;
    assert(st_start_inhibit(&snapshot, &clock, &out) == ST_INHIBITED);
    assert(memcmp(s_fram + 8, record, 96) == 0);
    check_sentinels_unmodified();
}

static void test_decoded_elapsed_is_not_time_evidence(void) {
    seed_golden_initial_fixture();
    StSnapshot prepared, waiting, out;
    StClock clock = {.epoch_ms = 1000000, .source = 1, .valid = true};
    StWaitCursor cursor;
    assert(st_load(&prepared) == ST_OK);
    assert(st_start_inhibit(&prepared, &clock, &waiting) == ST_OK);
    waiting.control.elapsed_ms = 1800000;
    assert(st_wait_open(&waiting, &clock, ST_NO_OLD_TIME_CREDIT, &cursor) == ST_OK);
    assert(st_finish_inhibit(&waiting, &cursor, &clock, &out) == ST_NOT_DUE);
    waiting.has_progress = true;
    waiting.progress.elapsed_ms = 1800000;
    waiting.control.duration_ms = 0;
    assert(st_wait_open(&waiting, &clock, ST_NO_OLD_TIME_CREDIT, &cursor) == ST_OK);
    assert(st_finish_inhibit(&waiting, &cursor, &clock, &out) == ST_NOT_DUE);
    check_sentinels_unmodified();
}

static void test_invalid_wait_entry_revokes_old_cursor(void) {
    seed_golden_initial_fixture();
    StSnapshot prepared, waiting, out;
    StClock clock = {.epoch_ms = 1000000, .source = 1, .valid = true};
    StWaitCursor old_cursor, rejected = {42};
    assert(st_load(&prepared) == ST_OK);
    assert(st_start_inhibit(&prepared, &clock, &waiting) == ST_OK);
    assert(st_wait_open(&waiting, &clock, ST_NO_OLD_TIME_CREDIT, &old_cursor) == ST_OK);
    StClock invalid = {.source = 1};
    assert(st_wait_open(&waiting, &invalid, ST_NO_OLD_TIME_CREDIT, &rejected) == ST_CLOCK_INVALID);
    assert(rejected.token == 0);
    clock.epoch_ms = 1800000;
    assert(st_finish_inhibit(&waiting, &old_cursor, &clock, &out) == ST_INVALID);
    invalid.valid = true;
    invalid.source = 4;
    assert(st_wait_open(&waiting, &invalid, ST_NO_OLD_TIME_CREDIT, &rejected) == ST_CLOCK_INVALID);
    assert(st_wait_open(&waiting, &clock, (StEntry)2, &rejected) == ST_INVALID);
    check_sentinels_unmodified();
}

static void test_wrapped_phase_sequences_and_adjacency_are_invalid(void) {
    StSnapshot snapshot;
    uint8_t record[96];
    for (unsigned phase = ST_PHASE_RETIRED; phase <= ST_PHASE_COMPLETE; phase++) {
        seed_golden_initial_fixture();
        StControl wrapped = {.arming = 1, .sequence = 0,
            .reservation = phase == ST_PHASE_COMPLETE ? UINT64_MAX - 2 : UINT64_MAX,
            .flags = 1, .phase = (uint8_t)phase, .fast_count = 7, .authorization = 1};
        helper_encode_critical(&wrapped, record);
        memcpy(s_fram + 8, record, 96);
        memcpy(s_fram + 104, record, 96);
        assert(st_load(&snapshot) == ST_INVALID);
    }
    for (unsigned reversed = 0; reversed < 2; reversed++) {
        seed_golden_initial_fixture();
        StControl last = {.arming = 1, .sequence = UINT64_MAX,
            .reservation = UINT64_MAX, .flags = 1, .phase = ST_PHASE_RESERVED,
            .fast_count = 7, .authorization = 1};
        helper_encode_critical(&last, record);
        memcpy(s_fram + (reversed ? 8 : 104), record, 96);
        assert(st_load(&snapshot) == ST_INVALID);
        check_sentinels_unmodified();
    }
}

static void test_checkpoint_ignores_decoded_selection_and_sequence(void) {
    seed_golden_initial_fixture();
    StSnapshot prepared, waiting, first, second;
    StClock clock = {.epoch_ms = 1000000, .source = 1, .valid = true};
    StWaitCursor cursor;
    assert(st_load(&prepared) == ST_OK);
    assert(st_start_inhibit(&prepared, &clock, &waiting) == ST_OK);
    assert(st_wait_open(&waiting, &clock, ST_NO_OLD_TIME_CREDIT, &cursor) == ST_OK);
    clock.epoch_ms += 60000;
    assert(st_checkpoint(&waiting, &cursor, &clock, &first) == ST_OK);
    uint8_t survivor[64];
    memcpy(survivor, s_fram + 200, 64);
    first.has_progress = false;
    first.progress_slot = 255;
    first.critical_slot = 255;
    first.progress.sequence = UINT64_MAX;
    first.control.arming = 42;
    clock.epoch_ms += 60000;
    assert(st_checkpoint(&first, &cursor, &clock, &second) == ST_OK);
    assert(second.progress.sequence == 2);
    assert(second.progress_slot == 1);
    assert(second.progress.elapsed_ms == 120000);
    assert(memcmp(survivor, s_fram + 200, 64) == 0);
    uint64_t elapsed = UINT64_MAX;
    s_fram[264 + 63] ^= 1; /* A competing byte change, even in an invalid slot, conflicts. */
    assert(st_wait_measure(&second, &cursor, &clock, &elapsed) == ST_CONFLICT);
    assert(elapsed == 120000);
    elapsed = UINT64_MAX;
    assert(st_wait_measure(&first, &cursor, &clock, &elapsed) == ST_CONFLICT);
    assert(elapsed == 120000);
    check_sentinels_unmodified();
}

static void test_fram_requires_wren_and_failed_wren_attempts_cleanup(void) {
    seed_golden_initial_fixture();
    uint8_t byte = 1;
    assert(!FRAM_writeData(200, &byte, 1));
    assert(s_fram[200] == 0);
    StSnapshot snapshot, out;
    StClock clock = {.source = 1, .valid = true};
    assert(st_load(&snapshot) == ST_OK);
    s_inject_wren_fail = true;
    assert(st_start_inhibit(&snapshot, &clock, &out) == ST_IO);
    assert(!s_wren_enabled);
    assert(s_wren_count == 1);
    assert(s_wrdi_count == 1);
    check_sentinels_unmodified();
}

/* These are actual written-byte positions, not write-call boundaries. */
typedef enum {
    CUT_START, CUT_REPAIR, CUT_CHECKPOINT, CUT_FINISH, CUT_ENTRY, CUT_PENDING_FINISH
} CutOperation;
static unsigned s_byte_cut_cases[6];

static void prepare_operation(CutOperation operation, StSnapshot *snapshot,
                              StWaitCursor *cursor, StClock *clock) {
    seed_golden_initial_fixture();
    *clock = (StClock){.epoch_ms = 1000000, .source = 1, .valid = true};
    assert(st_load(snapshot) == ST_OK);
    if (operation == CUT_START) {
        fram_faults_reset();
        return;
    }
    StSnapshot waiting, checkpoint;
    assert(st_start_inhibit(snapshot, clock, &waiting) == ST_OK);
    /* Keep ordinary-checkpoint coverage distinct from pending-entry coverage below. */
    assert(st_wait_open(&waiting, clock, ST_BOOT_CONTINUITY_PROVEN, cursor) == ST_OK);
    clock->epoch_ms += 60000;
    assert(st_checkpoint(&waiting, cursor, clock, &checkpoint) == ST_OK);
    *snapshot = checkpoint;
    if (operation == CUT_REPAIR) {
        memset(s_fram + 8, 0, 96); /* Critical B and sole progress A survive. */
        assert(st_load(snapshot) == ST_RECOVERABLE);
    } else if (operation == CUT_ENTRY || operation == CUT_PENDING_FINISH) {
        *clock = (StClock){.epoch_ms = 5000, .source = 2, .revision = 7, .valid = true};
        assert(st_wait_open(snapshot, clock, ST_NO_OLD_TIME_CREDIT, cursor) == ST_OK);
        clock->epoch_ms += operation == CUT_ENTRY ? 60000 : 1740000;
    } else {
        clock->epoch_ms += operation == CUT_FINISH ? 1740000 : 60000;
    }
    fram_faults_reset();
}

static StResult perform_operation(CutOperation operation, const StSnapshot *snapshot,
                                  StWaitCursor *cursor, const StClock *clock,
                                  StSnapshot *out) {
    switch (operation) {
    case CUT_START: return st_start_inhibit(snapshot, clock, out);
    case CUT_REPAIR: return st_repair(snapshot, out);
    case CUT_CHECKPOINT:
    case CUT_ENTRY: return st_checkpoint(snapshot, cursor, clock, out);
    case CUT_FINISH:
    case CUT_PENDING_FINISH: return st_finish_inhibit(snapshot, cursor, clock, out);
    }
    return ST_INVALID;
}

static void assert_conservative_recovery(CutOperation operation, const StClock *clock) {
    uint8_t retained[320];
    memcpy(retained, s_fram + 8, 320);
    fram_faults_reset(); /* Hardware restart retains every partially written byte. */
    StSnapshot recovered, repaired, finished;
    StResult result = st_load(&recovered); /* Public reset, no private reset hook. */
    assert(memcmp(retained, s_fram + 8, 320) == 0);
    assert(result == ST_OK || result == ST_RECOVERABLE);
    if (result == ST_RECOVERABLE) {
        assert(st_repair(&recovered, &repaired) == ST_OK);
        recovered = repaired;
    }
    if (operation == CUT_START) {
        assert(recovered.control.phase == ST_PHASE_PREPARED ||
               recovered.control.phase == ST_PHASE_INHIBIT);
    } else if (operation == CUT_FINISH || operation == CUT_PENDING_FINISH) {
        assert(recovered.control.phase == ST_PHASE_INHIBIT ||
               recovered.control.phase == ST_PHASE_ELIGIBLE);
    } else {
        assert(recovered.control.phase == ST_PHASE_INHIBIT);
    }
    if (recovered.control.phase == ST_PHASE_INHIBIT) {
        uint64_t durable = recovered.has_progress ? recovered.progress.elapsed_ms : 0;
        if (operation == CUT_START) assert(durable == 0);
        if (operation == CUT_REPAIR) assert(durable == 60000);
        if (operation == CUT_CHECKPOINT) assert(durable == 60000 || durable == 120000);
        if (operation == CUT_ENTRY) assert(durable == 60000);
        if (operation == CUT_FINISH || operation == CUT_PENDING_FINISH)
            assert(durable == 60000 || durable == 1800000);
        StWaitCursor fresh;
        uint64_t elapsed = UINT64_MAX;
        assert(st_wait_open(&recovered, clock, ST_NO_OLD_TIME_CREDIT, &fresh) == ST_OK);
        assert(st_wait_measure(&recovered, &fresh, clock, &elapsed) == ST_OK);
        assert(elapsed == durable); /* No off-time or unsaved interval invented. */
        assert(st_finish_inhibit(&recovered, &fresh, clock, &finished) ==
               (durable < 1800000 ? ST_NOT_DUE : ST_OK));
    }
    check_sentinels_unmodified();
}

static void test_every_partial_write_byte_survives_public_reset(void) {
    /* invalidation + payload + commit: critical=100, progress=68 bytes. */
    const int written_bytes[] = {200, 100, 68, 268, 68, 336};
    for (CutOperation operation = CUT_START; operation <= CUT_PENDING_FINISH; operation++) {
        for (int byte = 0; byte <= written_bytes[operation]; byte++) {
            StSnapshot snapshot, out;
            StWaitCursor cursor = {0};
            StClock clock;
            prepare_operation(operation, &snapshot, &cursor, &clock);
            s_byte_budget = byte;
            assert(perform_operation(operation, &snapshot, &cursor, &clock, &out) == ST_IO);
            assert(s_power_lost);
            if (operation >= CUT_CHECKPOINT) assert(cursor.token == 0);
            assert(s_wrdi_count == s_wren_count); /* Cleanup attempted even after the cut. */
            uint8_t partial[320];
            memcpy(partial, s_fram + 8, 320);
            assert(st_load(&out) == ST_IO); /* No readable-invalid fallback on device loss. */
            assert(memcmp(partial, s_fram + 8, 320) == 0);
            assert_conservative_recovery(operation, &clock);
            s_byte_cut_cases[operation]++;
        }
    }
}

static unsigned s_fault_cases;
static void test_each_hardware_failure_and_verified_noop_or_corruption(void) {
    for (CutOperation operation = CUT_START; operation <= CUT_PENDING_FINISH; operation++) {
        StSnapshot snapshot, out;
        StWaitCursor cursor = {0};
        StClock clock;
        prepare_operation(operation, &snapshot, &cursor, &clock);
        assert(perform_operation(operation, &snapshot, &cursor, &clock, &out) == ST_OK);
        uint8_t completed[320];
        memcpy(completed, out.raw, 320);
        const int calls[] = {s_read_count, s_write_count, s_wren_count,
                             s_wrdi_count, s_write_count, s_write_count};
        for (unsigned kind = 0; kind < 6; kind++) {
            for (int call = 1; call <= calls[kind]; call++) {
                prepare_operation(operation, &snapshot, &cursor, &clock);
                switch (kind) {
                case 0: s_fail_read_call = call; break;
                case 1: s_fail_write_call = call; break;
                case 2: s_fail_wren_call = call; break;
                case 3: s_fail_wrdi_call = call; break;
                case 4: s_noop_write_call = call; break;
                case 5: s_corrupt_write_call = call; break;
                }
                StResult result = perform_operation(operation, &snapshot, &cursor, &clock, &out);
                if (kind == 4 && result == ST_OK) {
                    /* Idempotent no-op (e.g. invalidating an empty slot) is harmless. */
                    assert(memcmp(out.raw, completed, 320) == 0);
                } else {
                    assert(result == ST_IO);
                    if (operation >= CUT_CHECKPOINT && s_write_count > 0) {
                        assert(cursor.token == 0);
                        uint64_t elapsed;
                        assert(st_wait_measure(&snapshot, &cursor, &clock, &elapsed) == ST_INVALID);
                    }
                }
                assert(s_wrdi_count == s_wren_count);
                assert_conservative_recovery(operation, &clock);
                s_fault_cases++;
            }
        }
    }
}

/* Independent literal wire fixtures, generated from contract offsets with Python zlib. */
static const uint8_t golden_inhibit[96] = {
    0x53, 0x54, 0x43, 0x33, 0x03, 0x00, 0x60, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x77, 0x1b, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x42, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xba, 0xe5, 0xe9, 0x94,
};
static const uint8_t golden_progress[64] = {
    0x53, 0x54, 0x50, 0x33, 0x03, 0x00, 0x40, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x60, 0xea, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa0, 0x2c, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xcb, 0x23, 0x43, 0x40,
};

static void wire_set_u64(uint8_t *record, unsigned offset, uint64_t value) {
    for (unsigned i = 0; i < 8; i++) record[offset + i] = (uint8_t)(value >> (8 * i));
}

static void wire_refresh_crc(uint8_t *record, unsigned crc_offset) {
    uint32_t crc = 0xffffffffU;
    for (unsigned i = 4; i < crc_offset; i++) {
        crc ^= record[i];
        for (unsigned bit = 0; bit < 8; bit++)
            crc = (crc >> 1) ^ ((crc & 1) ? 0xedb88320U : 0);
    }
    crc ^= 0xffffffffU;
    for (unsigned i = 0; i < 4; i++) record[crc_offset + i] = (uint8_t)(crc >> (8 * i));
}

static void seed_golden_wait_fixture(StSnapshot *snapshot) {
    seed_golden_initial_fixture();
    memcpy(s_fram + 8, golden_inhibit, 96);
    memcpy(s_fram + 104, golden_inhibit, 96);
    memcpy(s_fram + 200, golden_progress, 64);
    assert(st_load(snapshot) == ST_OK);
    assert(snapshot->has_progress);
    assert(snapshot->progress.elapsed_ms == 60000);
}

static void test_literal_wait_records_match_public_writes(void) {
    StSnapshot snapshot;
    StWaitCursor cursor;
    StClock clock;
    prepare_operation(CUT_CHECKPOINT, &snapshot, &cursor, &clock);
    assert(memcmp(s_fram + 8, golden_inhibit, 96) == 0);
    assert(memcmp(s_fram + 104, golden_inhibit, 96) == 0);
    assert(memcmp(s_fram + 200, golden_progress, 64) == 0);
    seed_golden_wait_fixture(&snapshot);
    check_sentinels_unmodified();
}

static void test_semantically_invalid_progress_cannot_supply_time(void) {
    for (unsigned variant = 0; variant < 10; variant++) {
        StSnapshot snapshot, out;
        seed_golden_wait_fixture(&snapshot);
        uint8_t *record = s_fram + 200;
        switch (variant) {
        case 0: wire_set_u64(record, 8, 2); break; /* Wrong arming. */
        case 1: wire_set_u64(record, 16, 2); break; /* Wrong wait. */
        case 2: wire_set_u64(record, 24, 0); break;
        case 3: wire_set_u64(record, 32, 1800001); break;
        case 4: record[48] = 4; break;
        case 5: record[52] = 2; break;
        case 6: record[53] = 1; break;
        case 7: record[52] = 0; record[48] = 0; break; /* Nonzero invalid reference. */
        case 8: record[52] = 0; wire_set_u64(record, 40, 0); break; /* Nonzero invalid source. */
        case 9: record[4] = 4; break;
        }
        wire_refresh_crc(record, 60);
        assert(st_load(&snapshot) == ST_OK);
        assert(!snapshot.has_progress);
        StClock clock = {.epoch_ms = 1060000, .source = 1, .valid = true};
        StWaitCursor cursor;
        uint64_t elapsed = UINT64_MAX;
        assert(st_wait_open(&snapshot, &clock, ST_NO_OLD_TIME_CREDIT, &cursor) == ST_OK);
        assert(st_wait_measure(&snapshot, &cursor, &clock, &elapsed) == ST_OK);
        assert(elapsed == 0);
        assert(st_finish_inhibit(&snapshot, &cursor, &clock, &out) == ST_NOT_DUE);
        check_sentinels_unmodified();
    }
}

static void test_matching_progress_survivor_is_preserved_in_either_slot(void) {
    for (unsigned slot = 0; slot < 2; slot++) {
        StSnapshot snapshot, out;
        seed_golden_wait_fixture(&snapshot);
        memcpy(s_fram + (slot ? 264 : 200), golden_progress, 64);
        uint8_t *wrong = s_fram + (slot ? 200 : 264);
        memcpy(wrong, golden_progress, 64);
        wire_set_u64(wrong, 16, 2);
        wire_set_u64(wrong, 24, UINT64_MAX);
        wire_set_u64(wrong, 32, 1800000);
        wire_refresh_crc(wrong, 60);
        assert(st_load(&snapshot) == ST_OK);
        assert(snapshot.progress_slot == slot);
        assert(snapshot.progress.elapsed_ms == 60000);
        StClock clock = {.epoch_ms = 1060000, .source = 1, .valid = true};
        StWaitCursor cursor;
        assert(st_wait_open(&snapshot, &clock, ST_NO_OLD_TIME_CREDIT, &cursor) == ST_OK);
        snapshot.has_progress = false;
        snapshot.progress_slot = (uint8_t)!slot;
        snapshot.progress.sequence = 0;
        clock.epoch_ms += 60000;
        s_byte_budget = 34;
        assert(st_checkpoint(&snapshot, &cursor, &clock, &out) == ST_IO);
        assert(memcmp(s_fram + (slot ? 264 : 200), golden_progress, 64) == 0);
        assert_conservative_recovery(CUT_CHECKPOINT, &clock);
    }
}

static void test_progress_disagreement_and_sequence_exhaustion(void) {
    StSnapshot snapshot, out;
    seed_golden_wait_fixture(&snapshot);
    memcpy(s_fram + 264, golden_progress, 64);
    wire_set_u64(s_fram + 264, 40, 1060001);
    wire_refresh_crc(s_fram + 264, 60);
    assert(st_load(&snapshot) == ST_INVALID); /* Equal sequence, different reference. */
    wire_set_u64(s_fram + 264, 24, 2);
    wire_set_u64(s_fram + 264, 32, 59999);
    wire_refresh_crc(s_fram + 264, 60);
    assert(st_load(&snapshot) == ST_INVALID); /* Newer elapsed must not decrease. */
    seed_golden_wait_fixture(&snapshot);
    wire_set_u64(s_fram + 200, 24, UINT64_MAX);
    wire_refresh_crc(s_fram + 200, 60);
    assert(st_load(&snapshot) == ST_OK);
    StClock clock = {.epoch_ms = 1060000, .source = 1, .valid = true};
    StWaitCursor cursor;
    assert(st_wait_open(&snapshot, &clock, ST_NO_OLD_TIME_CREDIT, &cursor) == ST_OK);
    snapshot.progress.sequence = 0; /* Decoded sequence cannot bypass exhaustion. */
    fram_faults_reset();
    assert(st_checkpoint(&snapshot, &cursor, &clock, &out) == ST_EXHAUSTED);
    clock.epoch_ms += 1740000;
    assert(st_finish_inhibit(&snapshot, &cursor, &clock, &out) == ST_EXHAUSTED);
    assert(s_write_count == 0);
    assert(st_load(&out) == ST_OK);
    assert(out.control.phase == ST_PHASE_INHIBIT);
    check_sentinels_unmodified();
}

static void test_reference_only_checkpoint_rebind_and_continuity(void) {
    StSnapshot snapshot, checkpoint, out;
    seed_golden_wait_fixture(&snapshot);
    StClock clock = {.epoch_ms = 5000, .source = 2, .revision = 7, .valid = true};
    StWaitCursor cursor, replaced;
    uint64_t elapsed;
    assert(st_wait_open(&snapshot, &clock, ST_NO_OLD_TIME_CREDIT, &cursor) == ST_OK);
    assert(st_checkpoint(&snapshot, &cursor, &clock, &checkpoint) == ST_OK);
    assert(checkpoint.progress.elapsed_ms == 60000);
    assert(checkpoint.progress.sequence == 2);
    assert(checkpoint.progress.reference_ms == 5000);
    assert(checkpoint.progress.source == 2);
    assert(st_wait_open(&snapshot, &clock, ST_NO_OLD_TIME_CREDIT, &replaced) == ST_CONFLICT);
    assert(st_wait_measure(&checkpoint, &cursor, &clock, &elapsed) == ST_INVALID);
    assert(st_load(&snapshot) == ST_OK);
    clock.epoch_ms += 10000;
    assert(st_wait_open(&snapshot, &clock, ST_BOOT_CONTINUITY_PROVEN, &cursor) == ST_OK);
    assert(st_wait_measure(&snapshot, &cursor, &clock, &elapsed) == ST_OK);
    assert(elapsed == 70000); /* Only time since new coherent reference is credited. */
    assert(st_checkpoint(&snapshot, &cursor, &clock, &checkpoint) == ST_OK);
    assert(checkpoint.progress.sequence == 3);
    clock.epoch_ms += 10000;
    assert(st_wait_measure(&checkpoint, &cursor, &clock, &elapsed) == ST_OK);
    assert(elapsed == 80000); /* Returned snapshot and cursor remain bound. */
    assert(st_wait_open(&checkpoint, &clock, ST_NO_OLD_TIME_CREDIT, &replaced) == ST_OK);
    assert(st_wait_measure(&checkpoint, &cursor, &clock, &elapsed) == ST_INVALID);
    assert(st_wait_measure(&checkpoint, &replaced, &clock, &elapsed) == ST_OK);
    assert(elapsed == 70000); /* Reentry discards unsaved 10 seconds. */
    assert(st_finish_inhibit(&checkpoint, &replaced, &clock, &out) == ST_NOT_DUE);
    check_sentinels_unmodified();
}

static void test_clock_errors_discard_unsaved_time_for_each_timed_api(void) {
    for (unsigned api = 0; api < 3; api++) {
        for (unsigned fault = 0; fault < 5; fault++) {
            StSnapshot snapshot, out;
            seed_golden_wait_fixture(&snapshot);
            StClock clock = {.epoch_ms = 1060000, .source = 1, .valid = true};
            StWaitCursor cursor;
            uint64_t elapsed;
            assert(st_wait_open(&snapshot, &clock, ST_NO_OLD_TIME_CREDIT, &cursor) == ST_OK);
            clock.epoch_ms += 10000;
            assert(st_wait_measure(&snapshot, &cursor, &clock, &elapsed) == ST_OK);
            assert(elapsed == 70000);
            StClock bad = clock;
            switch (fault) {
            case 0: bad.valid = false; break;
            case 1: bad.source = 0; break;
            case 2: bad.epoch_ms--; break;
            case 3: bad.source = 2; break;
            case 4: bad.revision++; break;
            }
            StResult result = api == 0 ? st_wait_measure(&snapshot, &cursor, &bad, &elapsed) :
                api == 1 ? st_checkpoint(&snapshot, &cursor, &bad, &out) :
                           st_finish_inhibit(&snapshot, &cursor, &bad, &out);
            assert(result == (fault < 2 ? ST_CLOCK_INVALID : ST_REANCHOR_REQUIRED));
            if (api == 0) assert(elapsed == 60000);
            assert(cursor.token == 0);
            assert(st_wait_measure(&snapshot, &cursor, &clock, &elapsed) == ST_INVALID);
            assert(st_load(&snapshot) == ST_OK);
            assert(st_wait_open(&snapshot, &clock, ST_NO_OLD_TIME_CREDIT, &cursor) == ST_OK);
            assert(st_wait_measure(&snapshot, &cursor, &clock, &elapsed) == ST_OK);
            assert(elapsed == 60000);
            check_sentinels_unmodified();
        }
    }
}

static void test_wrong_wait_cannot_be_finished_by_decoded_phase_edit(void) {
    seed_golden_initial_fixture();
    StControl retry = {.arming = 1, .sequence = 4, .reservation = 3, .wait = 4,
        .duration_ms = 172800000, .source = 1, .reference_valid = 1,
        .flags = 1, .phase = ST_PHASE_RETRY_WAIT, .fast_count = 1, .authorization = 1};
    uint8_t record[96];
    helper_encode_critical(&retry, record);
    memcpy(s_fram + 8, record, 96);
    memcpy(s_fram + 104, record, 96);
    StSnapshot snapshot, out;
    assert(st_load(&snapshot) == ST_OK);
    StClock clock = {.source = 1, .valid = true};
    StWaitCursor cursor;
    assert(st_wait_open(&snapshot, &clock, ST_NO_OLD_TIME_CREDIT, &cursor) == ST_OK);
    clock.epoch_ms = 172800000;
    snapshot.control.phase = ST_PHASE_INHIBIT;
    assert(st_finish_inhibit(&snapshot, &cursor, &clock, &out) == ST_INVALID);
    assert(memcmp(s_fram + 8, record, 96) == 0);
    check_sentinels_unmodified();
}

static void test_every_api_compares_all_raw_bytes_before_authority(void) {
    for (unsigned api = 0; api < 6; api++) {
        StSnapshot snapshot, out;
        StWaitCursor cursor = {0};
        StClock clock = {.epoch_ms = 1060000, .source = 1, .valid = true};
        if (api == 0) {
            seed_golden_initial_fixture();
            assert(st_load(&snapshot) == ST_OK);
        } else {
            seed_golden_wait_fixture(&snapshot);
            assert(st_wait_open(&snapshot, &clock, ST_NO_OLD_TIME_CREDIT, &cursor) == ST_OK);
        }
        fram_faults_reset();
        s_fram[327] ^= 1; /* Invalid unused progress B still participates in all-320 CAS. */
        uint64_t elapsed;
        StResult result = ST_INVALID;
        switch (api) {
        case 0: result = st_start_inhibit(&snapshot, &clock, &out); break;
        case 1: result = st_repair(&snapshot, &out); break;
        case 2: result = st_wait_open(&snapshot, &clock, ST_NO_OLD_TIME_CREDIT, &cursor); break;
        case 3: result = st_wait_measure(&snapshot, &cursor, &clock, &elapsed); break;
        case 4: result = st_checkpoint(&snapshot, &cursor, &clock, &out); break;
        case 5: result = st_finish_inhibit(&snapshot, &cursor, &clock, &out); break;
        }
        assert(result == ST_CONFLICT);
        assert(s_write_count == 0);
        check_sentinels_unmodified();
    }
}

static void test_start_and_repair_derive_metadata_from_raw(void) {
    seed_golden_initial_fixture();
    StSnapshot snapshot, waiting, repaired;
    StClock clock = {.source = 3, .valid = true}; /* Epoch zero is a valid sample. */
    assert(st_load(&snapshot) == ST_OK);
    snapshot.critical_mirrored = false;
    snapshot.critical_slot = 255;
    snapshot.control.arming = 42;
    snapshot.control.phase = ST_PHASE_COMPLETE;
    snapshot.control.sequence = UINT64_MAX;
    assert(st_start_inhibit(&snapshot, &clock, &waiting) == ST_OK);
    assert(waiting.control.arming == 1);
    assert(waiting.control.sequence == 1);
    assert(waiting.control.reference_valid == 1);
    assert(waiting.control.reference_ms == 0);
    memset(s_fram + 8, 0xff, 96);
    assert(st_load(&snapshot) == ST_RECOVERABLE);
    snapshot.critical_slot = 255;
    snapshot.critical_mirrored = true;
    snapshot.control.sequence = 99;
    snapshot.control.authorization = 0;
    assert(st_repair(&snapshot, &repaired) == ST_OK);
    assert(repaired.control.sequence == 1);
    assert(repaired.control.authorization == 1);
    assert(repaired.control.source == 3);
    assert(memcmp(repaired.raw, waiting.raw, 320) == 0);
    fram_faults_reset();
    assert(st_repair(&repaired, &snapshot) == ST_OK);
    assert(s_write_count == 0);
    assert(s_wren_count == 0);
    check_sentinels_unmodified();
}

static void test_entry_fallback_invalid_reference_and_elapsed_saturation(void) {
    for (unsigned variant = 0; variant < 5; variant++) {
        StSnapshot snapshot, checkpoint;
        seed_golden_wait_fixture(&snapshot);
        StClock clock = {.epoch_ms = 1060000, .source = 1, .valid = true};
        if (variant == 0) clock.epoch_ms--; /* Earlier than durable reference: no old credit. */
        if (variant == 1) clock.source = 3; /* Different reference domain: no old credit. */
        if (variant == 2) {
            s_fram[200 + 52] = 0;
            s_fram[200 + 48] = 0;
            wire_set_u64(s_fram + 200, 40, 0);
            wire_refresh_crc(s_fram + 200, 60);
            assert(st_load(&snapshot) == ST_OK);
            assert(snapshot.has_progress);
        }
        if (variant == 3 || variant == 4) {
            wire_set_u64(s_fram + 200, 40, 0);
            wire_refresh_crc(s_fram + 200, 60);
            assert(st_load(&snapshot) == ST_OK);
            clock.epoch_ms = variant == 3 ? UINT64_MAX : 0;
        }
        StWaitCursor cursor;
        uint64_t elapsed;
        assert(st_wait_open(&snapshot, &clock, ST_BOOT_CONTINUITY_PROVEN, &cursor) == ST_OK);
        assert(st_wait_measure(&snapshot, &cursor, &clock, &elapsed) == ST_OK);
        assert(elapsed == (variant == 3 ? 1800000 : 60000));
        if (variant == 4) {
            clock.epoch_ms = UINT64_MAX;
            assert(st_wait_measure(&snapshot, &cursor, &clock, &elapsed) == ST_OK);
            assert(elapsed == 1800000); /* elapsed + delta overflow saturates, never wraps. */
        }
        assert(st_checkpoint(&snapshot, &cursor, &clock, &checkpoint) == ST_OK);
        assert(checkpoint.progress.elapsed_ms == elapsed);
        assert(checkpoint.progress.reference_ms == clock.epoch_ms);
        assert(checkpoint.progress.reference_valid == 1);
        check_sentinels_unmodified();
    }
}

static void test_delayed_first_checkpoint_persists_entry_barrier(void) {
    StSnapshot snapshot, barrier, current;
    seed_golden_wait_fixture(&snapshot); /* Saved E=60,000, old reference=1,060,000. */
    StClock clock = {.epoch_ms = 5000, .source = 2, .revision = 7, .valid = true};
    StWaitCursor cursor;
    uint64_t elapsed;
    assert(st_wait_open(&snapshot, &clock, ST_NO_OLD_TIME_CREDIT, &cursor) == ST_OK);
    clock.epoch_ms += 30000;
    assert(st_wait_measure(&snapshot, &cursor, &clock, &elapsed) == ST_OK);
    assert(elapsed == 90000);
    clock.epoch_ms += 30000;
    assert(st_checkpoint(&snapshot, &cursor, &clock, &barrier) == ST_OK);
    assert(barrier.progress.elapsed_ms == 60000);
    assert(barrier.progress.reference_ms == 5000);
    assert(barrier.progress.source == 2);
    assert(barrier.progress.sequence == 2);
    assert(st_wait_measure(&barrier, &cursor, &clock, &elapsed) == ST_OK);
    assert(elapsed == 120000); /* Barrier must not discard the measured minute. */
    assert(st_checkpoint(&barrier, &cursor, &clock, &current) == ST_OK);
    assert(current.progress.elapsed_ms == 120000);
    assert(current.progress.reference_ms == 65000);
    assert(current.progress.sequence == 3);
    clock.epoch_ms += 10000;
    assert(st_wait_measure(&current, &cursor, &clock, &elapsed) == ST_OK);
    assert(elapsed == 130000); /* Neither reset to E nor double-added the minute. */
    check_sentinels_unmodified();
}

static void test_reset_after_entry_barrier_credits_anchor_once_or_saved_only(void) {
    for (unsigned proven = 0; proven < 2; proven++) {
        StSnapshot snapshot, barrier, current;
        seed_golden_wait_fixture(&snapshot);
        StClock clock = {.epoch_ms = 5000, .source = 2, .revision = 7, .valid = true};
        StWaitCursor cursor;
        uint64_t elapsed;
        assert(st_wait_open(&snapshot, &clock, ST_NO_OLD_TIME_CREDIT, &cursor) == ST_OK);
        clock.epoch_ms = 65000;
        assert(st_checkpoint(&snapshot, &cursor, &clock, &barrier) == ST_OK);
        assert(barrier.progress.elapsed_ms == 60000);
        assert(barrier.progress.reference_ms == 5000);
        assert(st_load(&snapshot) == ST_OK); /* Reset immediately after durable barrier. */
        assert(st_wait_measure(&barrier, &cursor, &clock, &elapsed) == ST_INVALID);
        if (!proven) clock = (StClock){.source = 3, .revision = 8, .valid = true};
        assert(st_wait_open(&snapshot, &clock,
               proven ? ST_BOOT_CONTINUITY_PROVEN : ST_NO_OLD_TIME_CREDIT, &cursor) == ST_OK);
        assert(st_wait_measure(&snapshot, &cursor, &clock, &elapsed) == ST_OK);
        assert(elapsed == (proven ? 120000 : 60000));
        clock.epoch_ms += 10000;
        assert(st_wait_measure(&snapshot, &cursor, &clock, &elapsed) == ST_OK);
        assert(elapsed == (proven ? 130000 : 70000));
        if (!proven) {
            assert(st_checkpoint(&snapshot, &cursor, &clock, &barrier) == ST_OK);
            assert(barrier.progress.elapsed_ms == 60000);
            assert(barrier.progress.reference_ms == 0);
            snapshot = barrier;
        }
        assert(st_checkpoint(&snapshot, &cursor, &clock, &current) == ST_OK);
        assert(current.progress.elapsed_ms == (proven ? 130000 : 70000));
        assert(current.progress.reference_ms == clock.epoch_ms);
        assert(st_load(&snapshot) == ST_OK);
        clock.epoch_ms += 10000;
        assert(st_wait_open(&snapshot, &clock, ST_BOOT_CONTINUITY_PROVEN, &cursor) == ST_OK);
        assert(st_wait_measure(&snapshot, &cursor, &clock, &elapsed) == ST_OK);
        assert(elapsed == (proven ? 140000 : 80000));
        check_sentinels_unmodified();
    }
}

static void test_pending_finish_exact_boundary_verifies_both_progress_records(void) {
    StSnapshot snapshot, out;
    StWaitCursor cursor;
    StClock clock;
    prepare_operation(CUT_PENDING_FINISH, &snapshot, &cursor, &clock);
    clock.epoch_ms--;
    assert(st_finish_inhibit(&snapshot, &cursor, &clock, &out) == ST_NOT_DUE);
    assert(s_write_count == 0);
    clock.epoch_ms++;
    assert(st_finish_inhibit(&snapshot, &cursor, &clock, &out) == ST_OK);
    assert(s_write_count == 12); /* Barrier, ordinary progress, both critical replicas. */
    assert(out.critical_mirrored);
    assert(out.control.phase == ST_PHASE_ELIGIBLE);
    assert(memcmp(out.raw, out.raw + 96, 96) == 0);
    uint8_t expected[64];
    memcpy(expected, golden_progress, 64);
    wire_set_u64(expected, 24, 2);
    wire_set_u64(expected, 40, 5000);
    expected[48] = 2;
    wire_refresh_crc(expected, 60);
    assert(memcmp(out.raw + 256, expected, 64) == 0); /* Entry barrier B. */
    wire_set_u64(expected, 24, 3);
    wire_set_u64(expected, 32, 1800000);
    wire_set_u64(expected, 40, 1745000);
    wire_refresh_crc(expected, 60);
    assert(memcmp(out.raw + 192, expected, 64) == 0); /* Current-time progress A. */
    assert(cursor.token == 0);
    assert(st_load(&snapshot) == ST_OK);
    assert(snapshot.control.phase == ST_PHASE_ELIGIBLE);
    check_sentinels_unmodified();
}

static void test_same_sample_barrier_and_checkpoint_do_not_manufacture_elapsed(void) {
    StSnapshot snapshot, barrier, current, out;
    seed_golden_wait_fixture(&snapshot);
    StClock clock = {.epoch_ms = 5000, .source = 2, .revision = 7, .valid = true};
    StWaitCursor cursor;
    uint64_t elapsed;
    assert(st_wait_open(&snapshot, &clock, ST_NO_OLD_TIME_CREDIT, &cursor) == ST_OK);
    assert(st_checkpoint(&snapshot, &cursor, &clock, &barrier) == ST_OK);
    assert(barrier.progress.elapsed_ms == 60000);
    assert(barrier.progress.reference_ms == 5000);
    assert(st_wait_measure(&barrier, &cursor, &clock, &elapsed) == ST_OK);
    assert(elapsed == 60000);
    assert(st_checkpoint(&barrier, &cursor, &clock, &current) == ST_OK);
    assert(current.progress.elapsed_ms == 60000);
    assert(current.progress.reference_ms == 5000);
    assert(current.progress.sequence == 3);
    assert(st_wait_measure(&current, &cursor, &clock, &elapsed) == ST_OK);
    assert(elapsed == 60000);
    fram_faults_reset();
    assert(st_finish_inhibit(&current, &cursor, &clock, &out) == ST_NOT_DUE);
    assert(s_write_count == 0);
    check_sentinels_unmodified();
}

static void test_barrier_then_current_sequence_exhaustion_never_wraps_or_authorizes(void) {
    for (unsigned headroom = 1; headroom <= 2; headroom++) {
        StSnapshot snapshot, out;
        seed_golden_wait_fixture(&snapshot);
        wire_set_u64(s_fram + 200, 24, UINT64_MAX - headroom);
        wire_refresh_crc(s_fram + 200, 60);
        assert(st_load(&snapshot) == ST_OK);
        StClock clock = {.epoch_ms = 5000, .source = 2, .revision = 7, .valid = true};
        StWaitCursor cursor;
        assert(st_wait_open(&snapshot, &clock, ST_NO_OLD_TIME_CREDIT, &cursor) == ST_OK);
        clock.epoch_ms += 1740000;
        fram_faults_reset();
        assert(st_finish_inhibit(&snapshot, &cursor, &clock, &out) ==
               (headroom == 1 ? ST_EXHAUSTED : ST_OK));
        assert(cursor.token == 0);
        assert(s_write_count == (headroom == 1 ? 3 : 12));
        assert(st_load(&snapshot) == ST_OK);
        if (headroom == 1) {
            assert(snapshot.control.phase == ST_PHASE_INHIBIT);
            assert(snapshot.progress.sequence == UINT64_MAX);
            assert(snapshot.progress.elapsed_ms == 60000);
            assert(snapshot.progress.reference_ms == 5000);
            assert(st_wait_open(&snapshot, &clock, ST_NO_OLD_TIME_CREDIT, &cursor) == ST_OK);
            fram_faults_reset();
            assert(st_checkpoint(&snapshot, &cursor, &clock, &out) == ST_EXHAUSTED);
            assert(s_write_count == 0);
        } else {
            assert(snapshot.control.phase == ST_PHASE_ELIGIBLE);
            uint8_t max_sequence[8];
            memset(max_sequence, 0xff, 8);
            assert(memcmp(snapshot.raw + 192 + 24, max_sequence, 8) == 0);
        }
        check_sentinels_unmodified();
    }
}

static void test_each_public_call_invalidates_on_permit(void) {
    enum { LOAD, REPAIR, START_INHIBIT, WAIT_OPEN, WAIT_MEASURE, CHECKPOINT,
           FINISH_INHIBIT, RESERVE, AFTER_OFF, RETIRE, COMPLETE, TAKE_NULL, CALL_COUNT };
    for (int call = 0; call < CALL_COUNT; call++) {
        StControl eligible = { .arming = 1, .sequence = 2, .flags = 1,
            .phase = ST_PHASE_ELIGIBLE, .authorization = 1 };
        uint8_t record[96];
        fram_model_reset();
        helper_encode_critical(&eligible, record);
        memcpy(&s_fram[8], record, 96); memcpy(&s_fram[104], record, 96);
        StSnapshot snapshot;
        StOnPermit original = {0}, copied;
        assert(st_load(&snapshot) == ST_OK);
        assert(st_reserve(&snapshot, NULL, NULL, &snapshot, &original) == ST_OK);
        copied = original;
        StClock clock = { .valid = false };
        StWaitCursor cursor = {0};
        uint64_t elapsed = 0;
        switch (call) {
        case LOAD: assert(st_load(NULL) == ST_INVALID); break;
        case REPAIR: assert(st_repair(NULL, NULL) == ST_INVALID); break;
        case START_INHIBIT: assert(st_start_inhibit(NULL, NULL, NULL) == ST_INVALID); break;
        case WAIT_OPEN: assert(st_wait_open(&snapshot, &clock, ST_NO_OLD_TIME_CREDIT, &cursor) == ST_CLOCK_INVALID); break;
        case WAIT_MEASURE: assert(st_wait_measure(NULL, NULL, NULL, &elapsed) == ST_INVALID); break;
        case CHECKPOINT: assert(st_checkpoint(NULL, NULL, NULL, NULL) == ST_INVALID); break;
        case FINISH_INHIBIT: assert(st_finish_inhibit(NULL, NULL, NULL, NULL) == ST_INVALID); break;
        case RESERVE: assert(st_reserve(NULL, NULL, NULL, NULL, NULL) == ST_INVALID); break;
        case AFTER_OFF: assert(st_after_off(NULL, NULL, NULL) == ST_INVALID); break;
        case RETIRE: assert(st_retire(NULL, NULL) == ST_INVALID); break;
        case COMPLETE: assert(st_complete(NULL, NULL) == ST_INVALID); break;
        case TAKE_NULL: assert(st_take_on_permit(NULL, NULL) == ST_INVALID); break;
        }
        StResult invalidated_take = st_take_on_permit(&snapshot, &original);
        if (invalidated_take != ST_INVALID) fprintf(stderr, "permit invalidation case %d returned %d\n", call, invalidated_take);
        assert(invalidated_take == ST_INVALID);
        /* A copied bearer token is also unusable after invalidation. */
        assert(st_take_on_permit(&snapshot, &copied) == ST_INVALID);
    }

    /* Any take attempt consumes before raw CAS/read; neither stale bytes nor I/O retry permit ON. */
    StControl eligible = { .arming = 1, .sequence = 2, .flags = 1,
        .phase = ST_PHASE_ELIGIBLE, .authorization = 1 };
    uint8_t record[96];
    StSnapshot snapshot, reserved, stale;
    StOnPermit permit, copied;
    fram_model_reset();
    helper_encode_critical(&eligible, record);
    memcpy(&s_fram[8], record, 96); memcpy(&s_fram[104], record, 96);
    assert(st_load(&snapshot) == ST_OK);
    assert(st_reserve(&snapshot, NULL, NULL, &reserved, &permit) == ST_OK);
    copied = permit;
    stale = reserved;
    stale.raw[319] ^= 1;
    assert(st_take_on_permit(&stale, &permit) == ST_CONFLICT);
    assert(st_take_on_permit(&reserved, &copied) == ST_INVALID);

    fram_model_reset();
    helper_encode_critical(&eligible, record);
    memcpy(&s_fram[8], record, 96); memcpy(&s_fram[104], record, 96);
    assert(st_load(&snapshot) == ST_OK);
    assert(st_reserve(&snapshot, NULL, NULL, &reserved, &permit) == ST_OK);
    copied = permit;
    fram_faults_reset();
    s_fail_read_call = 1; /* Full 320-byte CAS read fails. */
    assert(st_take_on_permit(&reserved, &permit) == ST_IO);
    fram_faults_reset();
    assert(st_take_on_permit(&reserved, &copied) == ST_INVALID);
}

static void test_reservation_permit_off_wait_and_retirement(void) {
    StControl eligible = { .arming = 1, .sequence = 2, .flags = 1,
        .phase = ST_PHASE_ELIGIBLE, .authorization = 1 };
    uint8_t record[96];
    fram_model_reset();
    helper_encode_critical(&eligible, record);
    memcpy(&s_fram[8], record, 96);
    memcpy(&s_fram[104], record, 96);
    StSnapshot snap, reserved, wait, retired, complete;
    StOnPermit permit, copied;
    assert(st_load(&snap) == ST_OK);
    StSnapshot stale = snap;
    stale.raw[319] ^= 0x01;
    assert(st_reserve(&stale, NULL, NULL, &reserved, &permit) == ST_CONFLICT);
    assert(permit.token == 0);
    snap.control.phase = ST_PHASE_COMPLETE; /* decoded caller fields are not authority */
    assert(st_reserve(&snap, NULL, NULL, &snap, &permit) == ST_OK); /* expected==out */
    reserved = snap;
    assert(reserved.control.phase == ST_PHASE_RESERVED);
    assert(reserved.control.sequence == 3 && reserved.control.fast_count == 1);
    copied = permit;
    reserved.control.phase = ST_PHASE_PREPARED;
    assert(st_take_on_permit(&reserved, &permit) == ST_OK);
    assert(st_take_on_permit(&reserved, &copied) == ST_INVALID);
    StClock clock = { .epoch_ms = 9000, .source = 1, .revision = 1, .valid = true };
    reserved.control.phase = ST_PHASE_COMPLETE;
    assert(st_after_off(&reserved, &clock, &reserved) == ST_OK);
    wait = reserved;
    assert(wait.control.phase == ST_PHASE_RETRY_WAIT);
    assert(wait.control.sequence == 4 && wait.control.wait == 4);
    assert(wait.control.duration_ms == 172800000ULL);
    wait.control.phase = ST_PHASE_COMPLETE;
    assert(st_retire(&wait, &wait) == ST_OK);
    retired = wait;
    assert(retired.control.phase == ST_PHASE_RETIRED && retired.control.sequence == 5);
    retired.control.phase = ST_PHASE_PREPARED;
    assert(st_complete(&retired, &retired) == ST_OK);
    complete = retired;
    assert(complete.control.phase == ST_PHASE_COMPLETE && complete.control.sequence == 6);
    assert(st_reserve(&complete, NULL, NULL, &snap, &permit) == ST_INVALID);
    assert(st_load(&snap) == ST_OK && snap.control.phase == ST_PHASE_COMPLETE);

    /* Repeated public-API reservations establish the seven-attempt threshold and weekly continuation. */
    eligible.sequence = 2; eligible.fast_count = 0;
    fram_model_reset();
    helper_encode_critical(&eligible, record);
    memcpy(&s_fram[8], record, 96); memcpy(&s_fram[104], record, 96);
    assert(st_load(&snap) == ST_OK);
    StWaitCursor cursor;
    clock.epoch_ms = 10000;
    assert(st_reserve(&snap, NULL, NULL, &reserved, &permit) == ST_OK);
    for (unsigned attempt = 1; attempt <= 8; attempt++) {
        assert(reserved.control.fast_count == (attempt < 7 ? attempt : 7));
        assert(st_take_on_permit(&reserved, &permit) == ST_OK);
        clock.epoch_ms++;
        assert(st_after_off(&reserved, &clock, &wait) == ST_OK);
        assert(wait.control.duration_ms == (attempt < 7 ? 172800000ULL : 604800000ULL));
        if (attempt == 8) break;
        assert(st_wait_open(&wait, &clock, ST_NO_OLD_TIME_CREDIT, &cursor) == ST_OK);
        clock.epoch_ms += wait.control.duration_ms - 1;
        assert(st_reserve(&wait, &cursor, &clock, &reserved, &permit) == ST_NOT_DUE);
        assert(permit.token == 0);
        clock.epoch_ms++;
        assert(st_reserve(&wait, &cursor, &clock, &reserved, &permit) == ST_OK);
    }
    assert(st_load(&snap) == ST_OK && snap.control.phase == ST_PHASE_RETRY_WAIT);
    assert(snap.control.fast_count == 7 && snap.control.duration_ms == 604800000ULL);

    /* Exhaust every byte boundary in a fresh ELIGIBLE reservation commit. */
    eligible.sequence = 2; eligible.fast_count = 0;
    for (int cut = 0; cut <= 201; cut++) {
        fram_model_reset();
        helper_encode_critical(&eligible, record);
        memcpy(&s_fram[8], record, 96); memcpy(&s_fram[104], record, 96);
        assert(st_load(&snap) == ST_OK);
        StSnapshot cut_out;
        StOnPermit cut_permit = {0};
        s_byte_budget = cut;
        StResult cut_result = st_reserve(&snap, NULL, NULL, &cut_out, &cut_permit);
        if (cut < 200) {
            assert(cut_result == ST_IO && cut_permit.token == 0);
            fram_faults_reset();
            StResult recovery = st_load(&snap);
            assert(recovery == ST_OK || recovery == ST_RECOVERABLE);
            if (snap.control.phase == ST_PHASE_RESERVED) {
                assert(recovery == ST_RECOVERABLE);
                assert(st_repair(&snap, &cut_out) == ST_OK);
                clock.epoch_ms += 1;
                assert(st_after_off(&cut_out, &clock, &wait) == ST_OK);
                assert(wait.control.reservation == 3 && wait.control.fast_count == 1);
            } else {
                assert(snap.control.phase == ST_PHASE_ELIGIBLE);
            }
            assert(st_take_on_permit(&cut_out, &cut_permit) != ST_OK);
        } else {
            assert(cut_result == (cut == 200 ? ST_IO : ST_OK));
            if (cut_result == ST_OK) assert(st_take_on_permit(&cut_out, &cut_permit) == ST_OK);
            else assert(cut_permit.token == 0);
        }
    }

    /* Byte-cut each remaining critical writer: OFF->wait, ELIGIBLE->RETIRED, RETIRED->COMPLETE. */
    for (unsigned operation = 0; operation < 3; operation++) {
        for (int cut = 0; cut <= 201; cut++) {
            fram_model_reset();
            helper_encode_critical(&eligible, record);
            memcpy(&s_fram[8], record, 96); memcpy(&s_fram[104], record, 96);
            assert(st_load(&snap) == ST_OK);
            if (operation == 0) {
                assert(st_reserve(&snap, NULL, NULL, &reserved, &permit) == ST_OK);
                assert(st_take_on_permit(&reserved, &permit) == ST_OK);
                clock.epoch_ms++;
            }
            if (operation == 2) {
                assert(st_retire(&snap, &retired) == ST_OK);
                snap = retired;
            }
            s_byte_budget = cut;
            StResult operation_result = operation == 0
                ? st_after_off(&reserved, &clock, &wait)
                : operation == 1 ? st_retire(&snap, &retired) : st_complete(&snap, &complete);
            if (cut <= 200) assert(operation_result == ST_IO);
            else assert(operation_result == ST_OK);
            fram_faults_reset();
            StResult recovery = st_load(&snap);
            assert(recovery == ST_OK || recovery == ST_RECOVERABLE);
            if (recovery == ST_RECOVERABLE) {
                assert(st_repair(&snap, &reserved) == ST_OK);
                snap = reserved;
            }
            if (operation == 0) {
                assert((snap.control.phase == ST_PHASE_RESERVED || snap.control.phase == ST_PHASE_RETRY_WAIT));
                assert(snap.control.reservation == 3 && snap.control.fast_count == 1);
                if (snap.control.phase == ST_PHASE_RESERVED)
                    assert(st_after_off(&snap, &clock, &wait) == ST_OK);
            } else if (operation == 1) {
                assert(snap.control.phase == ST_PHASE_ELIGIBLE || snap.control.phase == ST_PHASE_RETIRED);
            } else {
                assert(snap.control.phase == ST_PHASE_RETIRED || snap.control.phase == ST_PHASE_COMPLETE);
            }
        }
    }
    check_sentinels_unmodified();
}

/* #52: operation-entry counters exclude all public preparation calls. */
typedef enum {
    DOMAIN_RESERVE_ELIGIBLE, DOMAIN_RESERVE_RETRY, DOMAIN_RESERVE_PENDING,
    DOMAIN_AFTER_OFF, DOMAIN_RETIRE_ELIGIBLE, DOMAIN_RETIRE_RETRY,
    DOMAIN_COMPLETE, DOMAIN_TAKE, DOMAIN_OPERATION_COUNT
} DomainOperation;

typedef struct {
    StSnapshot before;
    StWaitCursor cursor;
    StClock clock;
    StOnPermit permit, old_permit;
} DomainScenario;

typedef struct {
    unsigned bytes, cuts, outside_endpoint, faults[6];
    int calls[4];
} DomainMatrixCounts;
static DomainMatrixCounts s_domain_counts[DOMAIN_OPERATION_COUNT];
static const char *const domain_names[] = {
    "A reserve_ELIGIBLE", "B reserve_due_RETRY_ordinary", "C reserve_due_RETRY_pending",
    "D after_OFF", "E retire_ELIGIBLE", "F retire_RETRY_WAIT", "G complete_RETIRED",
    "H take_fresh_permit"
};

static void seed_domain_eligible(StSnapshot *snapshot) {
    seed_golden_initial_fixture();
    StControl eligible = {.arming = 1, .sequence = 2, .flags = 1,
        .phase = ST_PHASE_ELIGIBLE, .authorization = 1};
    uint8_t record[96];
    helper_encode_critical(&eligible, record);
    memcpy(s_fram + 8, record, 96);
    memcpy(s_fram + 104, record, 96);
    memcpy(s_fram + 200, golden_progress, 64); /* An old inhibit wait, not retry credit. */
    assert(st_load(snapshot) == ST_OK);
}

static void prepare_domain_operation(DomainOperation operation, DomainScenario *scenario) {
    memset(scenario, 0, sizeof(*scenario));
    seed_domain_eligible(&scenario->before);
    scenario->clock = (StClock){.epoch_ms = 1000000, .source = 1, .revision = 1, .valid = true};
    StSnapshot reserved;
    assert(st_reserve(&scenario->before, NULL, NULL, &reserved, &scenario->permit) == ST_OK);
    scenario->old_permit = scenario->permit;
    if (operation == DOMAIN_TAKE) {
        scenario->before = reserved; /* Keep this fresh permit live until operation entry. */
    } else if (operation == DOMAIN_RESERVE_ELIGIBLE || operation == DOMAIN_RETIRE_ELIGIBLE) {
        seed_domain_eligible(&scenario->before); /* Deliberate independent fixture, public load revokes token. */
    } else {
        assert(st_after_off(&reserved, &scenario->clock, &scenario->before) == ST_OK);
        assert(scenario->before.control.wait == 4);
        assert(st_wait_open(&scenario->before, &scenario->clock,
                            ST_BOOT_CONTINUITY_PROVEN, &scenario->cursor) == ST_OK);
        scenario->clock.epoch_ms += 60000;
        assert(st_checkpoint(&scenario->before, &scenario->cursor, &scenario->clock,
                             &scenario->before) == ST_OK);
        assert(scenario->before.progress.wait == 4 && scenario->before.progress.sequence == 1);
        if (operation == DOMAIN_RESERVE_PENDING) {
            assert(st_load(&scenario->before) == ST_OK); /* Reboot: public reset only. */
            scenario->clock = (StClock){.epoch_ms = 5000, .source = 2, .revision = 7, .valid = true};
            assert(st_wait_open(&scenario->before, &scenario->clock,
                                ST_NO_OLD_TIME_CREDIT, &scenario->cursor) == ST_OK);
            scenario->clock.epoch_ms += 172740000;
        } else if (operation == DOMAIN_RESERVE_RETRY || operation == DOMAIN_AFTER_OFF) {
            scenario->clock.epoch_ms += 172740000;
            if (operation == DOMAIN_AFTER_OFF) {
                assert(st_reserve(&scenario->before, &scenario->cursor, &scenario->clock,
                                  &scenario->before, &scenario->permit) == ST_OK);
                scenario->old_permit = scenario->permit; /* OFF must revoke even a still-live token. */
                assert(scenario->before.control.reservation == 5);
                scenario->clock.epoch_ms += 5000;
            }
        } else if (operation == DOMAIN_COMPLETE) {
            assert(st_retire(&scenario->before, &scenario->before) == ST_OK);
        }
    }
    fram_faults_reset();
}

static StResult execute_domain_operation(DomainOperation operation, DomainScenario *scenario,
                                         StSnapshot *out, StOnPermit *issued) {
    switch (operation) {
    case DOMAIN_RESERVE_ELIGIBLE:
        return st_reserve(&scenario->before, NULL, NULL, out, issued);
    case DOMAIN_RESERVE_RETRY:
    case DOMAIN_RESERVE_PENDING:
        return st_reserve(&scenario->before, &scenario->cursor, &scenario->clock, out, issued);
    case DOMAIN_AFTER_OFF: return st_after_off(&scenario->before, &scenario->clock, out);
    case DOMAIN_RETIRE_ELIGIBLE:
    case DOMAIN_RETIRE_RETRY: return st_retire(&scenario->before, out);
    case DOMAIN_COMPLETE: return st_complete(&scenario->before, out);
    case DOMAIN_TAKE: return st_take_on_permit(&scenario->before, &scenario->permit);
    case DOMAIN_OPERATION_COUNT: break;
    }
    return ST_INVALID;
}

/* Independent contract wire oracle; never invoke the production codec. */
static void expected_domain_success(DomainOperation operation, const DomainScenario *scenario,
                                    uint8_t expected[320]) {
    memcpy(expected, scenario->before.raw, 320);
    if (operation == DOMAIN_TAKE) return;
    StControl next = {.arming = 1, .authorization = 1, .flags = 1};
    switch (operation) {
    case DOMAIN_RESERVE_ELIGIBLE:
        next.sequence = next.reservation = 3; next.fast_count = 1;
        next.phase = ST_PHASE_RESERVED; break;
    case DOMAIN_RESERVE_RETRY:
    case DOMAIN_RESERVE_PENDING:
        next.sequence = next.reservation = 5; next.fast_count = 2;
        next.phase = ST_PHASE_RESERVED; break;
    case DOMAIN_AFTER_OFF:
        next.sequence = next.wait = 6; next.reservation = 5; next.fast_count = 2;
        next.phase = ST_PHASE_RETRY_WAIT; next.duration_ms = 172800000;
        next.reference_ms = 173805000; next.source = 1; next.reference_valid = 1; break;
    case DOMAIN_RETIRE_ELIGIBLE:
        next.sequence = 3; next.phase = ST_PHASE_RETIRED; break;
    case DOMAIN_RETIRE_RETRY:
        next.sequence = 5; next.reservation = 3; next.fast_count = 1;
        next.phase = ST_PHASE_RETIRED; break;
    case DOMAIN_COMPLETE:
        next.sequence = 6; next.reservation = 3; next.fast_count = 1;
        next.phase = ST_PHASE_COMPLETE; break;
    case DOMAIN_TAKE:
    case DOMAIN_OPERATION_COUNT: assert(false); break;
    }
    helper_encode_critical(&next, expected);
    memcpy(expected + 96, expected, 96);
    if (operation == DOMAIN_RESERVE_RETRY || operation == DOMAIN_RESERVE_PENDING) {
        uint8_t progress[64];
        memcpy(progress, golden_progress, 64);
        wire_set_u64(progress, 16, 4);
        wire_set_u64(progress, 24, 2);
        if (operation == DOMAIN_RESERVE_PENDING) {
            wire_set_u64(progress, 40, 5000); progress[48] = 2;
            wire_refresh_crc(progress, 60);
            memcpy(expected + 256, progress, 64); /* B = no-old entry barrier. */
            wire_set_u64(progress, 24, 3);
        }
        wire_set_u64(progress, 32, 172800000);
        wire_set_u64(progress, 40, operation == DOMAIN_RESERVE_PENDING ? 172745000 : 173800000);
        wire_refresh_crc(progress, 60);
        memcpy(expected + (operation == DOMAIN_RESERVE_PENDING ? 192 : 256), progress, 64);
    }
}

static void assert_domain_no_replay(DomainOperation operation, DomainScenario *scenario,
                                    StResult result, StSnapshot *out, StOnPermit *issued) {
    if (operation <= DOMAIN_RESERVE_PENDING && result == ST_OK) {
        assert(issued->token != 0);
        StOnPermit copied = *issued;
        assert(st_take_on_permit(out, issued) == ST_OK);
        assert(st_take_on_permit(out, &copied) == ST_INVALID);
    } else if (operation <= DOMAIN_RESERVE_PENDING) {
        assert(issued->token == 0);
        assert(st_take_on_permit(&scenario->before, issued) == ST_INVALID);
    }
    if (operation == DOMAIN_TAKE) assert(scenario->permit.token == 0);
    assert(st_take_on_permit(&scenario->before, &scenario->old_permit) == ST_INVALID);
}

/* Recovery is judged by durable facts and fresh wait identities, not ST_OK alone. */
static void assert_domain_recovery(DomainOperation operation, const DomainScenario *scenario,
                                   const uint8_t expected[320]) {
    uint8_t retained[320];
    memcpy(retained, s_fram + 8, 320);
    fram_faults_reset(); /* Only modeled hardware resets here. */
    StSnapshot recovered, repaired;
    StResult loaded = st_load(&recovered); /* Public volatile reset. */
    assert(loaded == ST_OK || loaded == ST_RECOVERABLE);
    assert(memcmp(s_fram + 8, retained, 320) == 0);
    assert(memcmp(recovered.raw, retained, 320) == 0);
    bool successor_present = memcmp(retained, expected, 96) == 0 ||
                             memcmp(retained + 96, expected, 96) == 0;
    const uint8_t *selected = recovered.raw + recovered.critical_slot * 96;
    assert(memcmp(selected, successor_present ? expected : scenario->before.raw, 96) == 0);
    if (operation != DOMAIN_RESERVE_RETRY && operation != DOMAIN_RESERVE_PENDING) {
        assert(memcmp(retained + 192, scenario->before.raw + 192, 128) == 0);
    } else {
        if (operation == DOMAIN_RESERVE_RETRY)
            assert(memcmp(retained + 192, scenario->before.raw + 192, 64) == 0); /* Sole survivor A. */
        if (successor_present) {
            assert(memcmp(retained + 192, expected + 192, 128) == 0); /* Both timing prerequisites durable. */
        } else {
            assert(recovered.has_progress);
            unsigned offset = 192 + recovered.progress_slot * 64;
            bool saved = memcmp(retained + offset, scenario->before.raw + 192, 64) == 0;
            bool committed = memcmp(retained + offset, expected + offset, 64) == 0;
            assert(saved || committed); /* Exact saved, entry-barrier or due-current observation. */
        }
    }
    StOnPermit replay = scenario->permit;
    assert(st_take_on_permit(&recovered, &replay) == ST_INVALID);
    assert(st_repair(&recovered, &repaired) == ST_OK);
    assert(repaired.critical_mirrored);
    assert(memcmp(repaired.raw, selected, 96) == 0);
    assert(memcmp(repaired.raw, repaired.raw + 96, 96) == 0);
    assert(memcmp(repaired.raw + 192, retained + 192, 128) == 0);
    replay = scenario->permit;
    assert(st_take_on_permit(&repaired, &replay) == ST_INVALID); /* Repair cannot issue ON. */
    recovered = repaired;
    StClock clock = scenario->clock;
    clock.epoch_ms += 777;
    bool recovered_reservation = recovered.control.phase == ST_PHASE_RESERVED;
    if (recovered_reservation) {
        uint64_t consumed = recovered.control.reservation;
        uint8_t count = recovered.control.fast_count;
        assert(consumed == (operation == DOMAIN_AFTER_OFF ? 5U :
                            operation == DOMAIN_RESERVE_RETRY || operation == DOMAIN_RESERVE_PENDING ? 5U : 3U));
        assert(count == (consumed == 5 ? 2 : 1));
        assert(!recovered.has_progress);
        assert(st_after_off(&recovered, &clock, &recovered) == ST_OK);
        assert(recovered.control.reservation == consumed && recovered.control.fast_count == count);
        assert(recovered.control.wait == consumed + 1);
        assert(recovered.control.reference_ms == clock.epoch_ms);
        assert(!recovered.has_progress); /* Old full progress cannot shorten this new wait. */
        StSnapshot rebooted;
        assert(st_load(&rebooted) == ST_OK);
        assert(rebooted.control.reservation == consumed && rebooted.control.fast_count == count);
        recovered = rebooted; /* Same interrupted reservation consumed only once. */
    }
    if (recovered.control.phase == ST_PHASE_RETRY_WAIT) {
        uint64_t durable = recovered.has_progress ? recovered.progress.elapsed_ms : 0;
        uint64_t wait = recovered.control.wait, consumed = recovered.control.reservation;
        uint8_t count = recovered.control.fast_count;
        if (recovered_reservation || wait == 6) {
            assert(consumed == (wait == 6 ? 5U : 3U));
            assert(count == (wait == 6 ? 2 : 1) && durable == 0);
            assert(!recovered.has_progress);
        } else {
            assert(wait == 4 && consumed == 3 && count == 1);
            assert(recovered.has_progress && recovered.progress.wait == 4);
            if (operation == DOMAIN_RESERVE_RETRY || operation == DOMAIN_RESERVE_PENDING)
                assert(durable == 60000 || durable == 172800000);
            else assert(durable == 60000);
        }
        StWaitCursor fresh;
        StOnPermit permit;
        StSnapshot next;
        uint64_t elapsed = UINT64_MAX;
        assert(st_wait_open(&recovered, &clock, ST_NO_OLD_TIME_CREDIT, &fresh) == ST_OK);
        assert(st_wait_measure(&recovered, &fresh, &clock, &elapsed) == ST_OK);
        assert(elapsed == durable); /* No unsaved/off-time credit. */
        if (durable < 172800000) {
            fram_faults_reset();
            assert(st_reserve(&recovered, &fresh, &clock, &next, &permit) == ST_NOT_DUE);
            assert(permit.token == 0 && s_write_count == 0);
            clock.epoch_ms += 172800000 - durable - 1;
            assert(st_reserve(&recovered, &fresh, &clock, &next, &permit) == ST_NOT_DUE);
            assert(permit.token == 0 && s_write_count == 0);
            clock.epoch_ms++;
        }
        assert(st_reserve(&recovered, &fresh, &clock, &next, &permit) == ST_OK);
        assert(next.control.sequence == wait + 1 && next.control.reservation == wait + 1);
        assert(next.control.reservation != consumed && next.control.fast_count == count + 1);
        assert(st_take_on_permit(&next, &permit) == ST_OK); /* Only this explicitly fresh due reserve. */
    } else if (recovered.control.phase == ST_PHASE_RETIRED || recovered.control.phase == ST_PHASE_COMPLETE) {
        StOnPermit denied;
        StSnapshot out;
        assert(st_reserve(&recovered, NULL, NULL, &out, &denied) == ST_INVALID);
        assert(denied.token == 0);
    } else {
        assert(recovered.control.phase == ST_PHASE_ELIGIBLE);
    }
    check_sentinels_unmodified();
}

static void run_domain_matrix(DomainOperation operation) {
    DomainScenario scenario;
    StSnapshot out = {0};
    StOnPermit issued = {0};
    uint8_t expected[320];
    prepare_domain_operation(operation, &scenario);
    expected_domain_success(operation, &scenario, expected);
    assert(execute_domain_operation(operation, &scenario, &out, &issued) == ST_OK);
    DomainMatrixCounts *counts = &s_domain_counts[operation];
    counts->bytes = s_written_bytes;
    counts->calls[0] = s_read_count; counts->calls[1] = s_write_count;
    counts->calls[2] = s_wren_count; counts->calls[3] = s_wrdi_count;
    assert(s_wrdi_count == s_wren_count);
    assert(!s_wren_enabled);
    assert(memcmp(s_fram + 8, expected, 320) == 0);
    if (operation != DOMAIN_TAKE) assert(memcmp(out.raw, expected, 320) == 0);
    else assert(counts->bytes == 0 && counts->calls[1] == 0 && counts->calls[2] == 0 && counts->calls[3] == 0);
    assert_domain_no_replay(operation, &scenario, ST_OK, &out, &issued);

    /* An acknowledged mirrored successor survives loss of either replica, including retirement. */
    for (unsigned lost_slot = 0; lost_slot < 2; lost_slot++) {
        memcpy(s_fram + 8, expected, 320);
        memset(s_fram + 8 + lost_slot * 96, 0xff, 96);
        assert_domain_recovery(operation, &scenario, expected);
    }
    if (operation != DOMAIN_TAKE) {
        assert(counts->bytes > 0);
        for (unsigned cut = 0; cut <= counts->bytes + 1; cut++) {
            prepare_domain_operation(operation, &scenario);
            memset(&out, 0, sizeof(out)); issued.token = 0;
            s_byte_budget = (int)cut;
            StResult result = execute_domain_operation(operation, &scenario, &out, &issued);
            assert(result == (cut <= counts->bytes ? ST_IO : ST_OK));
            assert(s_power_lost == (cut <= counts->bytes));
            assert(s_written_bytes == (cut <= counts->bytes ? cut : counts->bytes));
            assert(s_wrdi_count == s_wren_count);
            assert_domain_no_replay(operation, &scenario, result, &out, &issued);
            if (s_power_lost) {
                uint8_t partial[320]; memcpy(partial, s_fram + 8, 320);
                assert(st_load(&out) == ST_IO); /* Device loss is not readable-invalid media. */
                assert(memcmp(partial, s_fram + 8, 320) == 0);
            }
            assert_domain_recovery(operation, &scenario, expected);
            if (cut <= counts->bytes) counts->cuts++;
            else counts->outside_endpoint++;
        }
    }
    for (unsigned kind = 0; kind < 6; kind++) {
        int calls = counts->calls[kind < 4 ? kind : 1];
        for (int call = 1; call <= calls; call++) {
            prepare_domain_operation(operation, &scenario);
            memset(&out, 0, sizeof(out)); issued.token = 0;
            switch (kind) {
            case 0: s_fail_read_call = call; break;
            case 1: s_fail_write_call = call; break;
            case 2: s_fail_wren_call = call; break;
            case 3: s_fail_wrdi_call = call; break;
            case 4: s_noop_write_call = call; break;
            case 5: s_corrupt_write_call = call; break;
            }
            StResult result = execute_domain_operation(operation, &scenario, &out, &issued);
            if (kind == 4 && result == ST_OK) {
                assert(memcmp(s_fram + 8, expected, 320) == 0); /* Only byte-identical no-op success. */
                assert(memcmp(out.raw, expected, 320) == 0);
            } else assert(result == ST_IO);
            assert(s_wrdi_count == s_wren_count);
            if (kind != 3) assert(!s_wren_enabled);
            if (kind == 0) assert(s_read_count == call);
            if (kind == 1) assert(s_write_count == call);
            if (kind == 2) assert(s_wren_count == call);
            if (kind == 3) assert(s_wrdi_count == call);
            if (operation == DOMAIN_TAKE)
                assert(s_written_bytes == 0 && s_write_count == 0 && s_wren_count == 0 && s_wrdi_count == 0);
            /* Includes final post-mirror full-snapshot read failure: committed successor, no permit. */
            if (kind == 0 && call == counts->calls[0]) assert(memcmp(s_fram + 8, expected, 320) == 0);
            assert_domain_no_replay(operation, &scenario, result, &out, &issued);
            assert_domain_recovery(operation, &scenario, expected);
            counts->faults[kind]++;
        }
    }
    unsigned faults = 0;
    for (unsigned kind = 0; kind < 6; kind++) faults += counts->faults[kind];
    printf("Domain matrix %s: bytes=%u cuts=%u outside=%u calls R/W/EN/DI=%d/%d/%d/%d "
           "faults R/W/EN/DI/noop/corrupt=%u/%u/%u/%u/%u/%u total=%u.\n",
           domain_names[operation], counts->bytes, counts->cuts, counts->outside_endpoint,
           counts->calls[0], counts->calls[1], counts->calls[2], counts->calls[3],
           counts->faults[0], counts->faults[1], counts->faults[2], counts->faults[3],
           counts->faults[4], counts->faults[5], faults);
}

static void test_reserve_eligible_transition_fault_matrix(void) { run_domain_matrix(DOMAIN_RESERVE_ELIGIBLE); }
static void test_reserve_due_retry_ordinary_fault_matrix(void) { run_domain_matrix(DOMAIN_RESERVE_RETRY); }
static void test_reserve_due_retry_pending_entry_fault_matrix(void) { run_domain_matrix(DOMAIN_RESERVE_PENDING); }
static void test_after_off_transition_fault_matrix(void) { run_domain_matrix(DOMAIN_AFTER_OFF); }
static void test_retire_eligible_transition_fault_matrix(void) { run_domain_matrix(DOMAIN_RETIRE_ELIGIBLE); }
static void test_retire_retry_wait_transition_fault_matrix(void) { run_domain_matrix(DOMAIN_RETIRE_RETRY); }
static void test_complete_retired_transition_fault_matrix(void) { run_domain_matrix(DOMAIN_COMPLETE); }
static void test_take_fresh_permit_read_fault_matrix(void) { run_domain_matrix(DOMAIN_TAKE); }

static void test_retry_saved_day_lost_clock_uncertain_reservation_and_two_weekly_retries(void) {
    StSnapshot snapshot, waiting, reserved;
    StOnPermit permit, uncertain;
    StWaitCursor cursor;
    StClock clock = {.epoch_ms = 1000000, .source = 1, .revision = 1, .valid = true};
    uint64_t elapsed;
    const uint64_t reservations[] = {3, 5, 7, 9, 11, 13, 15, 17, 19};
    seed_domain_eligible(&snapshot);
    assert(st_reserve(&snapshot, NULL, NULL, &reserved, &permit) == ST_OK);
    assert(reserved.control.reservation == reservations[0] && reserved.control.fast_count == 1);
    assert(st_take_on_permit(&reserved, &permit) == ST_OK);
    assert(st_after_off(&reserved, &clock, &waiting) == ST_OK);
    assert(waiting.control.wait == 4 && waiting.control.duration_ms == 172800000);
    assert(st_wait_open(&waiting, &clock, ST_BOOT_CONTINUITY_PROVEN, &cursor) == ST_OK);
    clock.epoch_ms += 86400000;
    assert(st_checkpoint(&waiting, &cursor, &clock, &waiting) == ST_OK);
    assert(waiting.progress.elapsed_ms == 86400000 && waiting.progress.wait == 4);
    fram_faults_reset();
    assert(st_load(&waiting) == ST_OK); /* Saved one verified day of the two-day wait. */
    clock = (StClock){.epoch_ms = 5000, .source = 2, .revision = 7, .valid = true};
    assert(st_wait_open(&waiting, &clock, ST_NO_OLD_TIME_CREDIT, &cursor) == ST_OK);
    assert(st_wait_measure(&waiting, &cursor, &clock, &elapsed) == ST_OK && elapsed == 86400000);
    clock.epoch_ms += 86399999;
    fram_faults_reset();
    assert(st_reserve(&waiting, &cursor, &clock, &reserved, &permit) == ST_NOT_DUE);
    assert(permit.token == 0 && s_write_count == 0);
    clock.epoch_ms++;
    assert(st_reserve(&waiting, &cursor, &clock, &reserved, &permit) == ST_OK);
    assert(reserved.control.reservation == reservations[1] && reserved.control.fast_count == 2);
    assert(reserved.control.sequence == 5 && cursor.token == 0);
    uncertain = permit; /* Reset without using ON authority: this reservation is uncertain, not replayable. */
    uint8_t old_progress[128];
    memcpy(old_progress, reserved.raw + 192, 128);
    fram_faults_reset();
    assert(st_load(&snapshot) == ST_OK);
    assert(snapshot.control.reservation == 5 && snapshot.control.fast_count == 2);
    assert(st_take_on_permit(&snapshot, &uncertain) == ST_INVALID);
    assert(st_load(&snapshot) == ST_OK);
    assert(snapshot.control.reservation == 5 && snapshot.control.fast_count == 2);
    clock.epoch_ms += 5000; /* Caller has commanded recovery OFF before creating cooldown. */
    assert(st_after_off(&snapshot, &clock, &waiting) == ST_OK);
    assert(waiting.control.wait == 6 && waiting.control.reservation == 5 && waiting.control.fast_count == 2);
    assert(waiting.control.reference_ms == clock.epoch_ms && !waiting.has_progress);
    assert(memcmp(waiting.raw + 192, old_progress, 128) == 0);
    assert(st_load(&waiting) == ST_OK);
    assert(st_after_off(&waiting, &clock, &snapshot) == ST_INVALID); /* No second consumption. */
    assert(waiting.control.fast_count == 2 && waiting.control.reservation == 5);
    clock = (StClock){.source = 3, .revision = 8, .valid = true};
    assert(st_wait_open(&waiting, &clock, ST_NO_OLD_TIME_CREDIT, &cursor) == ST_OK);
    assert(st_wait_measure(&waiting, &cursor, &clock, &elapsed) == ST_OK && elapsed == 0);
    assert(st_reserve(&waiting, &cursor, &clock, &reserved, &permit) == ST_NOT_DUE);
    clock.epoch_ms = 172799999;
    assert(st_reserve(&waiting, &cursor, &clock, &reserved, &permit) == ST_NOT_DUE);
    clock.epoch_ms++;
    assert(st_reserve(&waiting, &cursor, &clock, &reserved, &permit) == ST_OK);
    for (unsigned attempt = 3; attempt <= 9; attempt++) {
        assert(reserved.control.reservation == reservations[attempt - 1]);
        assert(reserved.control.sequence == reservations[attempt - 1]);
        assert(reserved.control.fast_count == (attempt < 7 ? attempt : 7));
        for (unsigned earlier = 0; earlier < attempt - 1; earlier++)
            assert(reserved.control.reservation != reservations[earlier]);
        assert(st_take_on_permit(&reserved, &permit) == ST_OK);
        clock.epoch_ms += 5000;
        assert(st_after_off(&reserved, &clock, &waiting) == ST_OK);
        assert(waiting.control.wait == reservations[attempt - 1] + 1);
        assert(waiting.control.reservation == reservations[attempt - 1]);
        assert(waiting.control.fast_count == (attempt < 7 ? attempt : 7));
        assert(waiting.control.duration_ms == (attempt < 7 ? 172800000ULL : 604800000ULL));
        assert(!waiting.has_progress);
        if (attempt == 9) break;
        assert(st_wait_open(&waiting, &clock, ST_NO_OLD_TIME_CREDIT, &cursor) == ST_OK);
        clock.epoch_ms += waiting.control.duration_ms - 1;
        fram_faults_reset();
        assert(st_reserve(&waiting, &cursor, &clock, &reserved, &permit) == ST_NOT_DUE);
        assert(permit.token == 0 && s_write_count == 0);
        clock.epoch_ms++;
        assert(st_reserve(&waiting, &cursor, &clock, &reserved, &permit) == ST_OK);
    }
    assert(st_load(&snapshot) == ST_OK);
    assert(snapshot.control.sequence == 20 && snapshot.control.reservation == 19);
    assert(snapshot.control.fast_count == 7 && snapshot.control.duration_ms == 604800000);
    check_sentinels_unmodified();
}

/* #53 adversarial detector coverage: not an ordinary power-cut/faithful-read fault. */
static unsigned s_adversarial_readback_cases, s_readback_alias_cases;

static void make_misleading_readback(const StSnapshot *before, const uint8_t expected[320],
                                     unsigned variant, uint8_t misleading[320]) {
    memcpy(misleading, variant == 0 ? before->raw : expected, 320);
    if (variant == 0 && !before->critical_mirrored) {
        /* Repair's historical predecessor is PREPARED, not its unmirrored input. */
        StControl prepared = {.arming = 1, .authorization = 1, .phase = ST_PHASE_PREPARED};
        helper_encode_critical(&prepared, misleading);
        memcpy(misleading + 96, misleading, 96);
    } else if (variant == 1) {
        StControl other = {.arming = 1, .authorization = 1, .flags = 1,
            .sequence = 9, .reservation = 9, .fast_count = 4, .phase = ST_PHASE_RESERVED};
        helper_encode_critical(&other, misleading);
        memcpy(misleading + 96, misleading, 96);
    } else if (variant == 2) {
        memcpy(misleading + 192, golden_progress, 64);
        wire_set_u64(misleading + 192, 40, 1060001);
        wire_refresh_crc(misleading + 192, 60);
    }
    assert(memcmp(misleading, expected, 320) != 0);
    /* Independent fixture validation proves the substitute is CRC-valid and mirrored. */
    memcpy(s_fram + 8, misleading, 320);
    StSnapshot decoded;
    assert(st_load(&decoded) == ST_OK);
    assert(decoded.critical_mirrored && memcmp(decoded.raw, misleading, 320) == 0);
}

static void test_domain_rejects_unintended_final_snapshot_readback(void) {
    unsigned failures = 0;
    for (DomainOperation operation = DOMAIN_RESERVE_ELIGIBLE; operation <= DOMAIN_COMPLETE; operation++) {
        DomainScenario scenario;
        StSnapshot normal;
        StOnPermit permit = {0};
        uint8_t expected[320];
        prepare_domain_operation(operation, &scenario);
        expected_domain_success(operation, &scenario, expected);
        assert(execute_domain_operation(operation, &scenario, &normal, &permit) == ST_OK);
        assert(memcmp(normal.raw, expected, 320) == 0);
        int final_read = s_read_count, writes = s_write_count;
        unsigned written_bytes = s_written_bytes;
        for (unsigned alias = 0; alias < 2; alias++) {
            for (unsigned variant = 0; variant < 3; variant++) {
                uint8_t misleading[320];
                prepare_domain_operation(operation, &scenario);
                make_misleading_readback(&scenario.before, expected, variant, misleading);
                prepare_domain_operation(operation, &scenario);
                StSnapshot out = scenario.before;
                StSnapshot *destination = alias ? &scenario.before : &out;
                uint8_t original_out[320]; memcpy(original_out, destination->raw, 320);
                memcpy(s_substitute_read_bytes, misleading, 320);
                s_substitute_read_call = final_read;
                permit.token = 0;
                StResult result = execute_domain_operation(operation, &scenario, destination, &permit);
                uint64_t issued_token = permit.token;
                assert(s_substituted_reads == 1 && s_read_count == final_read);
                assert(s_write_count == writes && s_written_bytes == written_bytes);
                assert(s_wrdi_count == s_wren_count && !s_wren_enabled);
                assert(memcmp(s_fram + 8, expected, 320) == 0); /* Only the read response lied. */
                bool rejected = result == ST_IO && issued_token == 0 &&
                    memcmp(destination->raw, original_out, 320) == 0;
                StOnPermit copied = permit;
                StResult take = st_take_on_permit(destination, &permit);
                rejected = rejected && take == ST_INVALID;
                assert(st_take_on_permit(destination, &copied) == ST_INVALID);
                assert(st_take_on_permit(destination, &scenario.old_permit) == ST_INVALID);
                if (operation == DOMAIN_RETIRE_RETRY) {
                    uint64_t elapsed;
                    rejected = rejected && st_wait_measure(&scenario.before, &scenario.cursor,
                                                           &scenario.clock, &elapsed) == ST_INVALID;
                }
                printf("Adversarial %s variant=%u alias=%u final_read=%d result=%d permit=%llu rejected=%u\n",
                       domain_names[operation], variant, alias, final_read, result,
                       (unsigned long long)issued_token, rejected);
                fflush(stdout);
                if (!rejected) failures++;
                fram_faults_reset();
                StSnapshot recovered;
                assert(st_load(&recovered) == ST_OK && memcmp(recovered.raw, expected, 320) == 0);
                assert(st_take_on_permit(&recovered, &copied) == ST_INVALID);
                check_sentinels_unmodified();
                s_adversarial_readback_cases++;
            }
        }
    }
    assert(failures == 0);
}

static void test_foundation_rejects_unintended_final_snapshot_readback(void) {
    const CutOperation operations[] = {CUT_START, CUT_REPAIR, CUT_CHECKPOINT, CUT_FINISH,
                                       CUT_ENTRY, CUT_PENDING_FINISH};
    const char *const names[] = {"start", "repair", "checkpoint", "finish", "entry", "pending_finish"};
    unsigned failures = 0;
    for (unsigned index = 0; index < sizeof(operations) / sizeof(operations[0]); index++) {
        CutOperation operation = operations[index];
        StSnapshot snapshot, normal;
        StWaitCursor cursor = {0};
        StClock clock;
        prepare_operation(operation, &snapshot, &cursor, &clock);
        assert(perform_operation(operation, &snapshot, &cursor, &clock, &normal) == ST_OK);
        int final_read = s_read_count, writes = s_write_count;
        unsigned written_bytes = s_written_bytes;
        uint8_t expected[320]; memcpy(expected, normal.raw, 320);
        for (unsigned alias = 0; alias < 2; alias++) {
            for (unsigned variant = 0; variant < 3; variant++) {
                uint8_t misleading[320];
                prepare_operation(operation, &snapshot, &cursor, &clock);
                make_misleading_readback(&snapshot, expected, variant, misleading);
                prepare_operation(operation, &snapshot, &cursor, &clock);
                StSnapshot out = snapshot;
                StSnapshot *destination = alias ? &snapshot : &out;
                uint8_t original_out[320]; memcpy(original_out, destination->raw, 320);
                memcpy(s_substitute_read_bytes, misleading, 320);
                s_substitute_read_call = final_read;
                StResult result = perform_operation(operation, &snapshot, &cursor, &clock, destination);
                assert(s_substituted_reads == 1 && s_read_count == final_read);
                assert(s_write_count == writes && s_written_bytes == written_bytes);
                assert(s_wrdi_count == s_wren_count && !s_wren_enabled);
                assert(memcmp(s_fram + 8, expected, 320) == 0);
                bool rejected = result == ST_IO && memcmp(destination->raw, original_out, 320) == 0;
                if (operation == CUT_CHECKPOINT || operation == CUT_ENTRY ||
                    operation == CUT_FINISH || operation == CUT_PENDING_FINISH) {
                    uint64_t elapsed;
                    StResult measured = st_wait_measure(destination, &cursor, &clock, &elapsed);
                    rejected = rejected && cursor.token == 0 && measured == ST_INVALID;
                }
                printf("Adversarial foundation %s variant=%u alias=%u final_read=%d result=%d rejected=%u\n",
                       names[index], variant, alias, final_read, result, rejected);
                fflush(stdout);
                if (!rejected) failures++;
                fram_faults_reset();
                StSnapshot recovered;
                assert(st_load(&recovered) == ST_OK && memcmp(recovered.raw, expected, 320) == 0);
                check_sentinels_unmodified();
                s_adversarial_readback_cases++;
            }
        }
    }
    assert(failures == 0);
}

static void test_intended_final_snapshot_preserves_normal_output_aliases(void) {
    for (DomainOperation operation = DOMAIN_RESERVE_ELIGIBLE; operation <= DOMAIN_COMPLETE; operation++) {
        DomainScenario scenario;
        uint8_t expected[320];
        StOnPermit permit = {0};
        prepare_domain_operation(operation, &scenario);
        expected_domain_success(operation, &scenario, expected);
        assert(execute_domain_operation(operation, &scenario, &scenario.before, &permit) == ST_OK);
        assert(memcmp(scenario.before.raw, expected, 320) == 0);
        assert(s_substituted_reads == 0);
        assert_domain_no_replay(operation, &scenario, ST_OK, &scenario.before, &permit);
        check_sentinels_unmodified();
        s_readback_alias_cases++;
    }
    const CutOperation operations[] = {CUT_START, CUT_REPAIR, CUT_CHECKPOINT, CUT_FINISH,
                                       CUT_ENTRY, CUT_PENDING_FINISH};
    for (unsigned index = 0; index < sizeof(operations) / sizeof(operations[0]); index++) {
        StSnapshot snapshot, normal;
        StWaitCursor cursor = {0};
        StClock clock;
        prepare_operation(operations[index], &snapshot, &cursor, &clock);
        assert(perform_operation(operations[index], &snapshot, &cursor, &clock, &normal) == ST_OK);
        prepare_operation(operations[index], &snapshot, &cursor, &clock);
        assert(perform_operation(operations[index], &snapshot, &cursor, &clock, &snapshot) == ST_OK);
        assert(memcmp(snapshot.raw, normal.raw, 320) == 0);
        assert(s_substituted_reads == 0);
        if (operations[index] == CUT_CHECKPOINT || operations[index] == CUT_ENTRY) {
            uint64_t elapsed;
            assert(st_wait_measure(&snapshot, &cursor, &clock, &elapsed) == ST_OK);
            assert(elapsed >= snapshot.progress.elapsed_ms);
        }
        check_sentinels_unmodified();
        s_readback_alias_cases++;
    }
}

int main(void) {
    RUN_FOUNDATION(test_each_public_call_invalidates_on_permit);
    RUN_FOUNDATION(test_reservation_permit_off_wait_and_retirement);
    RUN_FOUNDATION(test_reset_after_entry_barrier_credits_anchor_once_or_saved_only);
    RUN_FOUNDATION(test_pending_finish_exact_boundary_verifies_both_progress_records);
    RUN_FOUNDATION(test_same_sample_barrier_and_checkpoint_do_not_manufacture_elapsed);
    RUN_FOUNDATION(test_barrier_then_current_sequence_exhaustion_never_wraps_or_authorizes);
    RUN_FOUNDATION(test_delayed_first_checkpoint_persists_entry_barrier);
    RUN_FOUNDATION(test_every_api_compares_all_raw_bytes_before_authority);
    RUN_FOUNDATION(test_start_and_repair_derive_metadata_from_raw);
    RUN_FOUNDATION(test_entry_fallback_invalid_reference_and_elapsed_saturation);
    RUN_FOUNDATION(test_literal_wait_records_match_public_writes);
    RUN_FOUNDATION(test_semantically_invalid_progress_cannot_supply_time);
    RUN_FOUNDATION(test_matching_progress_survivor_is_preserved_in_either_slot);
    RUN_FOUNDATION(test_progress_disagreement_and_sequence_exhaustion);
    RUN_FOUNDATION(test_reference_only_checkpoint_rebind_and_continuity);
    RUN_FOUNDATION(test_clock_errors_discard_unsaved_time_for_each_timed_api);
    RUN_FOUNDATION(test_wrong_wait_cannot_be_finished_by_decoded_phase_edit);
    RUN_FOUNDATION(test_each_hardware_failure_and_verified_noop_or_corruption);
    RUN_FOUNDATION(test_every_partial_write_byte_survives_public_reset);
    RUN_FOUNDATION(test_fram_requires_wren_and_failed_wren_attempts_cleanup);
    RUN_FOUNDATION(test_checkpoint_ignores_decoded_selection_and_sequence);
    RUN_FOUNDATION(test_wrapped_phase_sequences_and_adjacency_are_invalid);
    RUN_FOUNDATION(test_invalid_wait_entry_revokes_old_cursor);
    RUN_FOUNDATION(test_decoded_elapsed_is_not_time_evidence);
    RUN_FOUNDATION(test_decoded_authorization_is_not_authority);
    printf("=== startup_persistence foundation tests ===\n");
    RUN_FOUNDATION(test_load_golden_fixture);
    RUN_FOUNDATION(test_unmirrored_recovery_and_repair);
    RUN_FOUNDATION(test_invalid_fixture_handling);
    RUN_FOUNDATION(test_io_and_driver_fault_injection);
    RUN_FOUNDATION(test_all_seven_phases_and_legal_pairs);
    RUN_FOUNDATION(test_start_inhibit_lifecycle);
    RUN_FOUNDATION(test_wait_open_measure_and_checkpoint);
    RUN_FOUNDATION(test_finish_inhibit_exact_boundary);
    RUN_FOUNDATION(test_power_cut_matrix);
    printf("Existing persistence coverage: %u scenario functions, %lu evaluated assertions; "
           "byte cuts start=%u repair=%u checkpoint=%u finish=%u entry=%u pending_finish=%u; "
           "hardware faults=%u.\n",
           s_foundation_scenarios, s_foundation_assertions, s_byte_cut_cases[0],
           s_byte_cut_cases[1], s_byte_cut_cases[2], s_byte_cut_cases[3],
           s_byte_cut_cases[4], s_byte_cut_cases[5], s_fault_cases);
    unsigned existing_scenarios = s_foundation_scenarios;
    unsigned long existing_assertions = s_foundation_assertions;
    RUN_FOUNDATION(test_reserve_eligible_transition_fault_matrix);
    RUN_FOUNDATION(test_reserve_due_retry_ordinary_fault_matrix);
    RUN_FOUNDATION(test_reserve_due_retry_pending_entry_fault_matrix);
    RUN_FOUNDATION(test_after_off_transition_fault_matrix);
    RUN_FOUNDATION(test_retire_eligible_transition_fault_matrix);
    RUN_FOUNDATION(test_retire_retry_wait_transition_fault_matrix);
    RUN_FOUNDATION(test_complete_retired_transition_fault_matrix);
    RUN_FOUNDATION(test_take_fresh_permit_read_fault_matrix);

    RUN_FOUNDATION(test_retry_saved_day_lost_clock_uncertain_reservation_and_two_weekly_retries);
    unsigned cuts = 0, endpoints = 0, faults = 0;
    for (unsigned operation = 0; operation < DOMAIN_OPERATION_COUNT; operation++) {
        cuts += s_domain_counts[operation].cuts;
        endpoints += s_domain_counts[operation].outside_endpoint;
        for (unsigned kind = 0; kind < 6; kind++) faults += s_domain_counts[operation].faults[kind];
    }
    printf("New domain coverage: %u scenario functions, %lu assertions; %u byte cuts, "
           "%u outside-cut successes, %u hardware faults, 16 acknowledged single-replica losses.\n",
           s_foundation_scenarios - existing_scenarios, s_foundation_assertions - existing_assertions,
           cuts, endpoints, faults);
    unsigned readback_start_scenarios = s_foundation_scenarios;
    unsigned long readback_start_assertions = s_foundation_assertions;
    RUN_FOUNDATION(test_domain_rejects_unintended_final_snapshot_readback);
    RUN_FOUNDATION(test_foundation_rejects_unintended_final_snapshot_readback);
    RUN_FOUNDATION(test_intended_final_snapshot_preserves_normal_output_aliases);
    printf("Readback correction: %u scenario functions, %lu assertions; %u adversarial final-read "
           "substitutions, %u normal output-alias successes.\n",
           s_foundation_scenarios - readback_start_scenarios,
           s_foundation_assertions - readback_start_assertions,
           s_adversarial_readback_cases, s_readback_alias_cases);
    printf("All persistence tests passed: %u scenario functions, %lu evaluated assertions.\n",
           s_foundation_scenarios, s_foundation_assertions);
    return 0;
}
