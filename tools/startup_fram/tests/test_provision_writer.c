/*
 * test_provision_writer.c
 *
 * Host tests for the real onboarding writer (onboard/startup_fram_provision.c)
 * against the memory FRAM device in fram_device_fake.c.
 *
 * The independent golden vector is the committed literal copy in
 * contract_fixture.h (taken from CONTRACT-VECTOR.json), so the suite is
 * self-contained and still fails if the C writer and the Python codec disagree.
 *
 * Failure model under test: a power cut or bus error at any write stage must
 * leave every committed (marker 0xA5) slot byte-identical to either the intended
 * record or a pre-existing committed record, must never write above address 47,
 * and must never report success unless the whole 48-byte image reads back.
 */

#include <string.h>

#include "contract_fixture.h"
#include "framework.h"
#include "fram_device_fake.h"
#include "startup_fram_provision.h"

#define IMAGE_SIZE 48u
#define SLOT_SIZE 20u
#define SLOT_A_ADDR 8u
#define SLOT_B_ADDR 28u
#define MARKER_IN_SLOT 19u

static uint8_t contract_image[IMAGE_SIZE];
static uint8_t contract_slot[SLOT_SIZE];
static uint8_t seq_max_image[IMAGE_SIZE];
static uint8_t seq_max_slot[SLOT_SIZE];

static void working_image(uint8_t image[IMAGE_SIZE]);

static uint8_t hex_nibble(char digit)
{
    if (digit >= '0' && digit <= '9') {
        return (uint8_t)(digit - '0');
    }
    if (digit >= 'a' && digit <= 'f') {
        return (uint8_t)(digit - 'a' + 10);
    }
    if (digit >= 'A' && digit <= 'F') {
        return (uint8_t)(digit - 'A' + 10);
    }
    return 0xFFu;
}

static size_t parse_hex(const char *text, uint8_t *out, size_t capacity)
{
    size_t written = 0;
    while (text[0] != '\0' && text[1] != '\0') {
        if (written >= capacity) {
            return 0u; /* longer than the caller's buffer */
        }
        const uint8_t high = hex_nibble(text[0]);
        const uint8_t low = hex_nibble(text[1]);
        if (high == 0xFFu || low == 0xFFu) {
            return 0u;
        }
        out[written++] = (uint8_t)((high << 4) | low);
        text += 2;
    }
    return written;
}

/* Test-local CRC32: an independent spelling of the contract, checked against the
 * golden slot below before it is used to build malformed records. */
static uint32_t test_crc32(const uint8_t *data, size_t length)
{
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t index = 0; index < length; index++) {
        crc ^= data[index];
        for (int bit = 0; bit < 8; bit++) {
            crc = (crc & 1u) ? ((crc >> 1) ^ 0xEDB88320u) : (crc >> 1);
        }
    }
    return crc ^ 0xFFFFFFFFu;
}

static void put_le32(uint8_t *target, uint32_t value)
{
    target[0] = (uint8_t)(value & 0xFFu);
    target[1] = (uint8_t)((value >> 8) & 0xFFu);
    target[2] = (uint8_t)((value >> 16) & 0xFFu);
    target[3] = (uint8_t)((value >> 24) & 0xFFu);
}

static uint32_t get_le32(const uint8_t *source)
{
    return (uint32_t)source[0] | ((uint32_t)source[1] << 8) | ((uint32_t)source[2] << 16)
           | ((uint32_t)source[3] << 24);
}

/* Build a committed slot with a correct CRC for the requested fields. */
static void make_slot(uint8_t slot[SLOT_SIZE], uint32_t sequence, uint8_t state,
                      uint8_t attempts, uint8_t flags, uint32_t remaining_ms)
{
    slot[0] = STARTUP_FRAM_TAG_0;
    slot[1] = STARTUP_FRAM_TAG_1;
    slot[2] = STARTUP_FRAM_TAG_2;
    slot[3] = STARTUP_FRAM_TAG_3;
    put_le32(&slot[STARTUP_FRAM_SLOT_SEQ_OFFSET], sequence);
    slot[STARTUP_FRAM_SLOT_STATE_OFFSET] = state;
    slot[STARTUP_FRAM_SLOT_ATTEMPTS_OFFSET] = attempts;
    slot[STARTUP_FRAM_SLOT_FLAGS_OFFSET] = flags;
    put_le32(&slot[STARTUP_FRAM_SLOT_REMAINING_OFFSET], remaining_ms);
    put_le32(&slot[STARTUP_FRAM_SLOT_CRC_OFFSET],
             test_crc32(slot, STARTUP_FRAM_SLOT_CRC_OFFSET));
    slot[STARTUP_FRAM_SLOT_MARKER_OFFSET] = STARTUP_FRAM_COMMIT_MARKER;
}

static void build_image(uint8_t image[IMAGE_SIZE], uint32_t boot_count, uint32_t adcs_mask,
                        const uint8_t slot_a[SLOT_SIZE], const uint8_t slot_b[SLOT_SIZE])
{
    put_le32(&image[STARTUP_FRAM_BOOT_COUNTER_ADDR], boot_count);
    put_le32(&image[STARTUP_FRAM_ADCS_MASK_ADDR], adcs_mask);
    memcpy(&image[STARTUP_FRAM_SLOT_A_ADDR], slot_a, SLOT_SIZE);
    memcpy(&image[STARTUP_FRAM_SLOT_B_ADDR], slot_b, SLOT_SIZE);
}

/* A fresh-image variant with a distinct ADCS mask and armed flag, used to prove
 * provisioning overwrites pre-existing records. */
static void working_image(uint8_t image[IMAGE_SIZE])
{
    uint8_t slot[SLOT_SIZE];
    make_slot(slot, 1u, STARTUP_FRAM_STATE_INITIAL_CHECKS, 0u, STARTUP_FRAM_FLAG_ARMED,
              STARTUP_FRAM_INHIBIT_MS);
    build_image(image, 7u, 0x00100001u, slot, slot);
}

/* ---- helpers shared by the failure-injection tests ---- */

static void seed_blank_device(void)
{
    fake_fram_reset();
}

/* Every committed slot on the device must be byte-identical to `expected`. */
static bool committed_slots_match(const uint8_t expected[IMAGE_SIZE])
{
    static const uint16_t slot_bases[2] = {SLOT_A_ADDR, SLOT_B_ADDR};
    for (int slot = 0; slot < 2; slot++) {
        const uint16_t base = slot_bases[slot];
        if (fake_fram_byte((uint16_t)(base + MARKER_IN_SLOT)) != STARTUP_FRAM_COMMIT_MARKER) {
            continue;
        }
        for (uint16_t offset = 0; offset < SLOT_SIZE; offset++) {
            if (fake_fram_byte((uint16_t)(base + offset)) != expected[base + offset]) {
                return false;
            }
        }
    }
    return true;
}

static bool device_image_is(const uint8_t expected[IMAGE_SIZE])
{
    for (uint16_t address = 0; address < IMAGE_SIZE; address++) {
        if (fake_fram_byte(address) != expected[address]) {
            return false;
        }
    }
    return true;
}

static unsigned writes_above_last_address(void)
{
    unsigned offending = 0;
    for (unsigned op = 0; op < fake_fram_write_ops(); op++) {
        const unsigned end = (unsigned)fake_fram_write_address(op)
                             + (unsigned)fake_fram_write_length(op);
        if (end > IMAGE_SIZE) {
            offending++;
        }
    }
    return offending;
}

/* ---- tests ---- */

static void test_golden_vector_parses(void)
{
    TEST_ASSERT(parse_hex(CONTRACT_IMAGE_HEX, contract_image, sizeof(contract_image))
                == IMAGE_SIZE);
    TEST_ASSERT(parse_hex(CONTRACT_SLOT_HEX, contract_slot, sizeof(contract_slot))
                == SLOT_SIZE);
    TEST_ASSERT(memcmp(&contract_image[SLOT_A_ADDR], contract_slot, SLOT_SIZE) == 0);
    TEST_ASSERT(memcmp(&contract_image[SLOT_B_ADDR], contract_slot, SLOT_SIZE) == 0);
    /* The CRC helper must agree with the golden slot before it builds bad records. */
    TEST_ASSERT(get_le32(&contract_slot[STARTUP_FRAM_SLOT_CRC_OFFSET])
                == test_crc32(contract_slot, STARTUP_FRAM_SLOT_CRC_OFFSET));
    TEST_ASSERT(get_le32(&contract_slot[STARTUP_FRAM_SLOT_REMAINING_OFFSET])
                == STARTUP_FRAM_INHIBIT_MS);
    TEST_ASSERT(get_le32(&contract_image[STARTUP_FRAM_BOOT_COUNTER_ADDR]) == 0u);
    TEST_ASSERT(get_le32(&contract_image[STARTUP_FRAM_ADCS_MASK_ADDR]) == 0u);
    TEST_ASSERT(startup_fram_image_is_valid(contract_image, IMAGE_SIZE));
}

static void test_validation_accepts_golden_and_rejects_bounds(void)
{
    TEST_ASSERT(startup_fram_image_is_valid(contract_image, IMAGE_SIZE));
    TEST_ASSERT(!startup_fram_image_is_valid(NULL, IMAGE_SIZE));
    TEST_ASSERT(!startup_fram_image_is_valid(contract_image, IMAGE_SIZE - 1u));
    TEST_ASSERT(!startup_fram_image_is_valid(contract_image, IMAGE_SIZE + 1u));
    TEST_ASSERT(!startup_fram_image_is_valid(contract_image, 0u));
}

static void test_validation_rejects_record_corruption(void)
{
    uint8_t image[IMAGE_SIZE];

    memcpy(image, contract_image, sizeof(image));
    image[STARTUP_FRAM_SLOT_A_ADDR + 0] = 'X'; /* tag */
    TEST_ASSERT(!startup_fram_image_is_valid(image, IMAGE_SIZE));

    memcpy(image, contract_image, sizeof(image));
    image[STARTUP_FRAM_SLOT_A_ADDR + STARTUP_FRAM_SLOT_CRC_OFFSET] ^= 0x01u;
    TEST_ASSERT(!startup_fram_image_is_valid(image, IMAGE_SIZE));

    memcpy(image, contract_image, sizeof(image));
    image[STARTUP_FRAM_SLOT_B_ADDR + STARTUP_FRAM_SLOT_MARKER_OFFSET] =
        STARTUP_FRAM_INVALID_MARKER;
    TEST_ASSERT(!startup_fram_image_is_valid(image, IMAGE_SIZE));

    /* flags bit1 is not defined */
    memcpy(image, contract_image, sizeof(image));
    {
        uint8_t slot[SLOT_SIZE];
        make_slot(slot, 1u, STARTUP_FRAM_STATE_INITIAL_CHECKS, 0u, 0x02u,
                  STARTUP_FRAM_INHIBIT_MS);
        memcpy(&image[SLOT_A_ADDR], slot, SLOT_SIZE);
    }
    TEST_ASSERT(!startup_fram_image_is_valid(image, IMAGE_SIZE));
}

static void test_validation_rejects_boot_and_adcs_out_of_range(void)
{
    uint8_t image[IMAGE_SIZE];

    put_le32(&image[STARTUP_FRAM_BOOT_COUNTER_ADDR], STARTUP_FRAM_BOOT_COUNT_MAX + 1u);
    put_le32(&image[STARTUP_FRAM_ADCS_MASK_ADDR], 0u);
    memcpy(&image[SLOT_A_ADDR], contract_slot, SLOT_SIZE);
    memcpy(&image[SLOT_B_ADDR], contract_slot, SLOT_SIZE);
    TEST_ASSERT(!startup_fram_image_is_valid(image, IMAGE_SIZE));

    put_le32(&image[STARTUP_FRAM_BOOT_COUNTER_ADDR], STARTUP_FRAM_BOOT_COUNT_MAX);
    TEST_ASSERT(startup_fram_image_is_valid(image, IMAGE_SIZE));

    /* reserved ADCS bits 21..31 */
    put_le32(&image[STARTUP_FRAM_ADCS_MASK_ADDR], 0x00200000u);
    TEST_ASSERT(!startup_fram_image_is_valid(image, IMAGE_SIZE));
    put_le32(&image[STARTUP_FRAM_ADCS_MASK_ADDR], 0x001FFFFFu);
    TEST_ASSERT(startup_fram_image_is_valid(image, IMAGE_SIZE));
}

static void test_validation_rejects_state_and_field_ranges(void)
{
    uint8_t image[IMAGE_SIZE];
    uint8_t slot_a[SLOT_SIZE];
    uint8_t slot_b[SLOT_SIZE];

    /* transient states 4, 5, 7 and non-MCUState values */
    const uint8_t bad_states[] = {4u, 5u, 7u, 8u};
    for (size_t index = 0; index < sizeof(bad_states); index++) {
        make_slot(slot_a, 1u, bad_states[index], 1u, STARTUP_FRAM_FLAG_ARMED, 0u);
        memcpy(slot_b, slot_a, SLOT_SIZE);
        build_image(image, 0u, 0u, slot_a, slot_b);
        TEST_ASSERT(!startup_fram_image_is_valid(image, IMAGE_SIZE));
    }

    /* attempts above the limit */
    make_slot(slot_a, 1u, STARTUP_FRAM_STATE_WAITING_FOR_UPLINK,
              STARTUP_FRAM_ATTEMPT_LIMIT + 1u, STARTUP_FRAM_FLAG_ARMED,
              STARTUP_FRAM_RETRY_MS_SATURATED);
    memcpy(slot_b, slot_a, SLOT_SIZE);
    build_image(image, 0u, 0u, slot_a, slot_b);
    TEST_ASSERT(!startup_fram_image_is_valid(image, IMAGE_SIZE));

    /* INITIAL_CHECKS: attempts must be 0 and remaining <= 1800000 */
    make_slot(slot_a, 1u, STARTUP_FRAM_STATE_INITIAL_CHECKS, 1u, 0u, STARTUP_FRAM_INHIBIT_MS);
    memcpy(slot_b, slot_a, SLOT_SIZE);
    build_image(image, 0u, 0u, slot_a, slot_b);
    TEST_ASSERT(!startup_fram_image_is_valid(image, IMAGE_SIZE));

    make_slot(slot_a, 1u, STARTUP_FRAM_STATE_INITIAL_CHECKS, 0u, 0u,
              STARTUP_FRAM_INHIBIT_MS + 1u);
    memcpy(slot_b, slot_a, SLOT_SIZE);
    build_image(image, 0u, 0u, slot_a, slot_b);
    TEST_ASSERT(!startup_fram_image_is_valid(image, IMAGE_SIZE));

    /* WAIT_POWER_ON requires remaining_ms 0 */
    make_slot(slot_a, 1u, STARTUP_FRAM_STATE_WAIT_POWER_ON, 1u, STARTUP_FRAM_FLAG_ARMED, 1u);
    memcpy(slot_b, slot_a, SLOT_SIZE);
    build_image(image, 0u, 0u, slot_a, slot_b);
    TEST_ASSERT(!startup_fram_image_is_valid(image, IMAGE_SIZE));

    /* WAITING_FOR_UPLINK: attempts 1..7 and remaining bounded by the retry interval */
    make_slot(slot_a, 1u, STARTUP_FRAM_STATE_WAITING_FOR_UPLINK, 0u, STARTUP_FRAM_FLAG_ARMED,
              STARTUP_FRAM_RETRY_MS);
    memcpy(slot_b, slot_a, SLOT_SIZE);
    build_image(image, 0u, 0u, slot_a, slot_b);
    TEST_ASSERT(!startup_fram_image_is_valid(image, IMAGE_SIZE));

    make_slot(slot_a, 1u, STARTUP_FRAM_STATE_WAITING_FOR_UPLINK, 1u, STARTUP_FRAM_FLAG_ARMED,
              STARTUP_FRAM_RETRY_MS + 1u);
    memcpy(slot_b, slot_a, SLOT_SIZE);
    build_image(image, 0u, 0u, slot_a, slot_b);
    TEST_ASSERT(!startup_fram_image_is_valid(image, IMAGE_SIZE));

    make_slot(slot_a, 1u, STARTUP_FRAM_STATE_WAITING_FOR_UPLINK, 1u, STARTUP_FRAM_FLAG_ARMED,
              STARTUP_FRAM_RETRY_MS);
    memcpy(slot_b, slot_a, SLOT_SIZE);
    build_image(image, 0u, 0u, slot_a, slot_b);
    TEST_ASSERT(startup_fram_image_is_valid(image, IMAGE_SIZE));

    /* the saturated count allows the week-long interval and anything shorter */
    make_slot(slot_a, 1u, STARTUP_FRAM_STATE_WAITING_FOR_UPLINK,
              STARTUP_FRAM_ATTEMPT_LIMIT, STARTUP_FRAM_FLAG_ARMED, STARTUP_FRAM_RETRY_MS);
    memcpy(slot_b, slot_a, SLOT_SIZE);
    build_image(image, 0u, 0u, slot_a, slot_b);
    TEST_ASSERT(startup_fram_image_is_valid(image, IMAGE_SIZE));

    make_slot(slot_a, 1u, STARTUP_FRAM_STATE_WAITING_FOR_UPLINK,
              STARTUP_FRAM_ATTEMPT_LIMIT, STARTUP_FRAM_FLAG_ARMED,
              STARTUP_FRAM_RETRY_MS_SATURATED);
    memcpy(slot_b, slot_a, SLOT_SIZE);
    build_image(image, 0u, 0u, slot_a, slot_b);
    TEST_ASSERT(startup_fram_image_is_valid(image, IMAGE_SIZE));

    /* SAT_TO_GROUND_1 and MAIN_OPS persist without an outstanding wait */
    make_slot(slot_a, 1u, STARTUP_FRAM_STATE_SAT_TO_GROUND_1, 1u, STARTUP_FRAM_FLAG_ARMED, 1u);
    memcpy(slot_b, slot_a, SLOT_SIZE);
    build_image(image, 0u, 0u, slot_a, slot_b);
    TEST_ASSERT(!startup_fram_image_is_valid(image, IMAGE_SIZE));

    make_slot(slot_a, 1u, STARTUP_FRAM_STATE_MAIN_OPS, 1u, STARTUP_FRAM_FLAG_ARMED, 1u);
    memcpy(slot_b, slot_a, SLOT_SIZE);
    build_image(image, 0u, 0u, slot_a, slot_b);
    TEST_ASSERT(!startup_fram_image_is_valid(image, IMAGE_SIZE));

    /* non-initial states require the armed flag */
    make_slot(slot_a, 1u, STARTUP_FRAM_STATE_MAIN_OPS, 1u, 0u, 0u);
    memcpy(slot_b, slot_a, SLOT_SIZE);
    build_image(image, 0u, 0u, slot_a, slot_b);
    TEST_ASSERT(!startup_fram_image_is_valid(image, IMAGE_SIZE));

    make_slot(slot_a, 1u, STARTUP_FRAM_STATE_MAIN_OPS, 1u, STARTUP_FRAM_FLAG_ARMED, 0u);
    memcpy(slot_b, slot_a, SLOT_SIZE);
    build_image(image, 0u, 0u, slot_a, slot_b);
    TEST_ASSERT(startup_fram_image_is_valid(image, IMAGE_SIZE));

    /* sequence 0 is never valid; 0xFFFFFFFF is the last publishable record */
    make_slot(slot_a, 0u, STARTUP_FRAM_STATE_INITIAL_CHECKS, 0u, 0u, STARTUP_FRAM_INHIBIT_MS);
    memcpy(slot_b, slot_a, SLOT_SIZE);
    build_image(image, 0u, 0u, slot_a, slot_b);
    TEST_ASSERT(!startup_fram_image_is_valid(image, IMAGE_SIZE));

    make_slot(slot_a, STARTUP_FRAM_SEQ_MAX, STARTUP_FRAM_STATE_INITIAL_CHECKS, 0u, 0u,
              STARTUP_FRAM_INHIBIT_MS);
    memcpy(slot_b, slot_a, SLOT_SIZE);
    build_image(image, 0u, 0u, slot_a, slot_b);
    TEST_ASSERT(startup_fram_image_is_valid(image, IMAGE_SIZE));
}

static void test_validation_rejects_ambiguous_equal_sequences(void)
{
    uint8_t image[IMAGE_SIZE];
    uint8_t slot_a[SLOT_SIZE];
    uint8_t slot_b[SLOT_SIZE];

    /* identical sequence, different content */
    make_slot(slot_a, 1u, STARTUP_FRAM_STATE_INITIAL_CHECKS, 0u, 0u, STARTUP_FRAM_INHIBIT_MS);
    make_slot(slot_b, 1u, STARTUP_FRAM_STATE_INITIAL_CHECKS, 0u, STARTUP_FRAM_FLAG_ARMED,
              STARTUP_FRAM_INHIBIT_MS);
    build_image(image, 0u, 0u, slot_a, slot_b);
    TEST_ASSERT(!startup_fram_image_is_valid(image, IMAGE_SIZE));

    /* different sequences: the newer record wins, older content may differ */
    make_slot(slot_b, 2u, STARTUP_FRAM_STATE_MAIN_OPS, 1u, STARTUP_FRAM_FLAG_ARMED, 0u);
    build_image(image, 0u, 0u, slot_a, slot_b);
    TEST_ASSERT(startup_fram_image_is_valid(image, IMAGE_SIZE));
}

static void test_sequence_max_fixture_is_provisionable(void)
{
    /* 0xFFFFFFFF is the last legally published sequence: the loader accepts it and
     * only the next save reports exhaustion, so the writer must accept it too. */
    TEST_ASSERT(parse_hex(SEQ_MAX_IMAGE_HEX, seq_max_image, sizeof(seq_max_image))
                == IMAGE_SIZE);
    TEST_ASSERT(parse_hex(SEQ_MAX_SLOT_HEX, seq_max_slot, sizeof(seq_max_slot))
                == SLOT_SIZE);
    TEST_ASSERT(memcmp(&seq_max_image[SLOT_A_ADDR], seq_max_slot, SLOT_SIZE) == 0);
    TEST_ASSERT(memcmp(&seq_max_image[SLOT_B_ADDR], seq_max_slot, SLOT_SIZE) == 0);
    TEST_ASSERT(get_le32(&seq_max_slot[STARTUP_FRAM_SLOT_SEQ_OFFSET])
                == STARTUP_FRAM_SEQ_MAX);
    TEST_ASSERT(startup_fram_image_is_valid(seq_max_image, IMAGE_SIZE));

    seed_blank_device();
    TEST_ASSERT(startup_fram_provision_image(seq_max_image, IMAGE_SIZE)
                == STARTUP_FRAM_PROVISION_OK);
    TEST_ASSERT(device_image_is(seq_max_image));

    /* Sequence 0 stays invalid, so nothing else about the range changed. */
    make_slot(seq_max_slot, 0u, STARTUP_FRAM_STATE_MAIN_OPS, 1u, STARTUP_FRAM_FLAG_ARMED, 0u);
    build_image(seq_max_image, 0u, 0u, seq_max_slot, seq_max_slot);
    TEST_ASSERT(!startup_fram_image_is_valid(seq_max_image, IMAGE_SIZE));
}

static void test_rejected_image_touches_nothing(void)
{
    uint8_t image[IMAGE_SIZE];
    memcpy(image, contract_image, sizeof(image));
    image[STARTUP_FRAM_SLOT_B_ADDR + 5] ^= 0xFFu; /* CRC no longer matches */

    seed_blank_device();
    TEST_ASSERT(startup_fram_provision_image(image, IMAGE_SIZE)
                == STARTUP_FRAM_PROVISION_IMAGE_REJECTED);
    TEST_ASSERT(fake_fram_write_ops() == 0u);
    TEST_ASSERT(fake_fram_read_ops() == 0u);
    TEST_ASSERT(!fake_fram_out_of_range());

    TEST_ASSERT(startup_fram_provision_image(NULL, IMAGE_SIZE)
                == STARTUP_FRAM_PROVISION_IMAGE_REJECTED);
    TEST_ASSERT(startup_fram_provision_image(contract_image, IMAGE_SIZE - 1u)
                == STARTUP_FRAM_PROVISION_IMAGE_REJECTED);
    TEST_ASSERT(startup_fram_provision_image(contract_image, IMAGE_SIZE + 1u)
                == STARTUP_FRAM_PROVISION_IMAGE_REJECTED);
    TEST_ASSERT(fake_fram_write_ops() == 0u);
}

static void test_success_publishes_exact_golden_image(void)
{
    uint8_t image[IMAGE_SIZE];

    parse_hex(CONTRACT_IMAGE_HEX, image, sizeof(image));
    seed_blank_device();
    fake_fram_fill(0xAAu); /* marks every byte the writer does not touch */

    TEST_ASSERT(startup_fram_provision_image(image, IMAGE_SIZE)
                == STARTUP_FRAM_PROVISION_OK);
    TEST_ASSERT(device_image_is(image));
    TEST_ASSERT(!fake_fram_out_of_range());
    TEST_ASSERT(writes_above_last_address() == 0u);
    /* addresses 48.. stay untouched, so nothing outside 0..47 was written */
    for (uint16_t address = IMAGE_SIZE; address < 64u; address++) {
        TEST_ASSERT(fake_fram_byte(address) == 0xAAu);
    }
}

static void test_write_order_invalidates_markers_first_and_commits_last(void)
{
    uint8_t image[IMAGE_SIZE];
    parse_hex(CONTRACT_IMAGE_HEX, image, sizeof(image));

    seed_blank_device();
    TEST_ASSERT(startup_fram_provision_image(image, IMAGE_SIZE)
                == STARTUP_FRAM_PROVISION_OK);

    const unsigned ops = fake_fram_write_ops();
    TEST_ASSERT(ops == 6u);
    /* operations 0 and 1 clear the two commit markers */
    TEST_ASSERT(fake_fram_write_address(0) == SLOT_A_ADDR + MARKER_IN_SLOT);
    TEST_ASSERT(fake_fram_write_length(0) == 1u);
    TEST_ASSERT(fake_fram_write_address(1) == SLOT_B_ADDR + MARKER_IN_SLOT);
    TEST_ASSERT(fake_fram_write_length(1) == 1u);
    /* operation 2 is the 27-byte prefix: boot counter, ADCS mask, slot A payload */
    TEST_ASSERT(fake_fram_write_address(2) == 0u);
    TEST_ASSERT(fake_fram_write_length(2) == 27u);
    /* operation 3 is the 19-byte slot B payload without its marker */
    TEST_ASSERT(fake_fram_write_address(3) == SLOT_B_ADDR);
    TEST_ASSERT(fake_fram_write_length(3) == 19u);
    /* operations 4 and 5 publish the commit markers last */
    TEST_ASSERT(fake_fram_write_address(4) == SLOT_A_ADDR + MARKER_IN_SLOT);
    TEST_ASSERT(fake_fram_write_length(4) == 1u);
    TEST_ASSERT(fake_fram_write_address(5) == SLOT_B_ADDR + MARKER_IN_SLOT);
    TEST_ASSERT(fake_fram_write_length(5) == 1u);
}

static void test_provisioning_replaces_existing_records(void)
{
    uint8_t image[IMAGE_SIZE];
    uint8_t old_slot[SLOT_SIZE];
    uint8_t old_image[IMAGE_SIZE];

    working_image(image);
    /* A published old record: sequence 3, mid-handshake, armed. */
    make_slot(old_slot, 3u, STARTUP_FRAM_STATE_WAITING_FOR_UPLINK, 2u,
              STARTUP_FRAM_FLAG_ARMED, STARTUP_FRAM_RETRY_MS / 2u);
    build_image(old_image, 12u, 0x00001001u, old_slot, old_slot);

    fake_fram_reset();
    fake_fram_seed(0u, old_image, IMAGE_SIZE);

    TEST_ASSERT(startup_fram_provision_image(image, IMAGE_SIZE)
                == STARTUP_FRAM_PROVISION_OK);
    TEST_ASSERT(device_image_is(image));
    TEST_ASSERT(!committed_slots_match(old_image));
}

/* Power cut / bus error at every stage and at every byte offset of a block:
 * only complete records may ever be committed, never a hybrid. */
static void test_every_stage_failure_never_publishes_a_partial_record(void)
{
    /* Staged write lengths, in order: two marker invalidations, the 0..26 prefix,
     * the 28..46 slot B payload, then the two commit markers. */
    static const uint16_t op_lengths[6] = {1u, 1u, 27u, 19u, 1u, 1u};
    const FakeFramFault faults[] = {
        FAKE_FAULT_HARD_ERROR,     FAKE_FAULT_SHORT_WRITE,
        FAKE_FAULT_CORRUPT_BYTE,   FAKE_FAULT_LOST_RESPONSE,
        FAKE_FAULT_READ_ERROR,     FAKE_FAULT_WRDI_ERROR,
    };
    uint8_t image[IMAGE_SIZE];
    parse_hex(CONTRACT_IMAGE_HEX, image, sizeof(image));

    for (unsigned op = 0; op < 6u; op++) {
        for (size_t fault = 0; fault < sizeof(faults) / sizeof(faults[0]); fault++) {
            for (unsigned detail = 0; detail <= 27u; detail++) {
                fake_fram_reset();
                fake_fram_fill(0xAAu);
                fake_fram_fail_write(op, faults[fault], detail);

                const StartupFramProvisionStatus status =
                    startup_fram_provision_image(image, IMAGE_SIZE);
                if (status == STARTUP_FRAM_PROVISION_OK) {
                    /* A short write longer than the block is not a fault at all. */
                    TEST_ASSERT(faults[fault] == FAKE_FAULT_SHORT_WRITE);
                    TEST_ASSERT(detail >= op_lengths[op]);
                    TEST_ASSERT(device_image_is(image));
                }
                TEST_ASSERT(committed_slots_match(image));
                TEST_ASSERT(!fake_fram_out_of_range());
                TEST_ASSERT(writes_above_last_address() == 0u);
            }
        }
    }
}

static void test_failure_reasons_are_reported(void)
{
    uint8_t image[IMAGE_SIZE];
    parse_hex(CONTRACT_IMAGE_HEX, image, sizeof(image));

    /* hard error on the first marker invalidation */
    fake_fram_reset();
    fake_fram_fail_write(0u, FAKE_FAULT_HARD_ERROR, 0u);
    TEST_ASSERT(startup_fram_provision_image(image, IMAGE_SIZE)
                == STARTUP_FRAM_PROVISION_INVALIDATE_FAILED);

    /* second marker invalidation fails: slot A marker is already cleared */
    fake_fram_reset();
    fake_fram_fail_write(1u, FAKE_FAULT_HARD_ERROR, 0u);
    TEST_ASSERT(startup_fram_provision_image(image, IMAGE_SIZE)
                == STARTUP_FRAM_PROVISION_INVALIDATE_FAILED);
    TEST_ASSERT(fake_fram_byte(SLOT_A_ADDR + MARKER_IN_SLOT) == STARTUP_FRAM_INVALID_MARKER);

    /* short write inside the boot counter/ADCS/prefix block */
    fake_fram_reset();
    fake_fram_fail_write(2u, FAKE_FAULT_SHORT_WRITE, 2u);
    TEST_ASSERT(startup_fram_provision_image(image, IMAGE_SIZE)
                == STARTUP_FRAM_PROVISION_WRITE_FAILED);
    TEST_ASSERT(fake_fram_byte(SLOT_A_ADDR + MARKER_IN_SLOT) == STARTUP_FRAM_INVALID_MARKER);
    TEST_ASSERT(fake_fram_byte(SLOT_B_ADDR + MARKER_IN_SLOT) == STARTUP_FRAM_INVALID_MARKER);

    /* corrupt one bit of the slot B payload */
    fake_fram_reset();
    fake_fram_fail_write(3u, FAKE_FAULT_CORRUPT_BYTE, 5u);
    TEST_ASSERT(startup_fram_provision_image(image, IMAGE_SIZE)
                == STARTUP_FRAM_PROVISION_WRITE_FAILED);

    /* read-back failure while verifying the payload */
    fake_fram_reset();
    fake_fram_fail_write(3u, FAKE_FAULT_READ_ERROR, 2u);
    TEST_ASSERT(startup_fram_provision_image(image, IMAGE_SIZE) != STARTUP_FRAM_PROVISION_OK);
    TEST_ASSERT(fake_fram_byte(SLOT_A_ADDR + MARKER_IN_SLOT) == STARTUP_FRAM_INVALID_MARKER);
    TEST_ASSERT(fake_fram_byte(SLOT_B_ADDR + MARKER_IN_SLOT) == STARTUP_FRAM_INVALID_MARKER);

    /* write-disable failure leaves the update uncertain */
    fake_fram_reset();
    fake_fram_fail_write(2u, FAKE_FAULT_WRDI_ERROR, 0u);
    TEST_ASSERT(startup_fram_provision_image(image, IMAGE_SIZE)
                == STARTUP_FRAM_PROVISION_WRITE_FAILED);

    /* write-enable failure: nothing is applied at all */
    fake_fram_reset();
    fake_fram_fill(0xAAu);
    fake_fram_fail_write_enable(0u);
    TEST_ASSERT(startup_fram_provision_image(image, IMAGE_SIZE)
                == STARTUP_FRAM_PROVISION_INVALIDATE_FAILED);
    TEST_ASSERT(fake_fram_byte(SLOT_A_ADDR + MARKER_IN_SLOT) == 0xAAu);

    /* marker committed but the response was lost on the last stage */
    fake_fram_reset();
    fake_fram_fail_write(5u, FAKE_FAULT_LOST_RESPONSE, 0u);
    TEST_ASSERT(startup_fram_provision_image(image, IMAGE_SIZE)
                == STARTUP_FRAM_PROVISION_COMMIT_FAILED);
    TEST_ASSERT(device_image_is(image)); /* complete new state, unconfirmed */
}

static void test_old_record_survives_interrupted_invalidation(void)
{
    uint8_t image[IMAGE_SIZE];
    uint8_t old_slot[SLOT_SIZE];
    uint8_t old_image[IMAGE_SIZE];

    working_image(image);
    make_slot(old_slot, 5u, STARTUP_FRAM_STATE_MAIN_OPS, 3u, STARTUP_FRAM_FLAG_ARMED, 0u);
    build_image(old_image, 4u, 0x00000003u, old_slot, old_slot);

    fake_fram_reset();
    fake_fram_seed(0u, old_image, IMAGE_SIZE);
    /* Slot A's marker is cleared, then slot B's invalidation fails. */
    fake_fram_fail_write(1u, FAKE_FAULT_HARD_ERROR, 0u);

    TEST_ASSERT(startup_fram_provision_image(image, IMAGE_SIZE)
                == STARTUP_FRAM_PROVISION_INVALIDATE_FAILED);
    TEST_ASSERT(fake_fram_byte(SLOT_A_ADDR + MARKER_IN_SLOT) == STARTUP_FRAM_INVALID_MARKER);
    /* Slot B is still the previous complete record, not a hybrid. */
    TEST_ASSERT(memcmp(fake_fram_memory() + SLOT_B_ADDR, old_slot, SLOT_SIZE) == 0);
    TEST_ASSERT(committed_slots_match(old_image) || committed_slots_match(image));
}

int main(void)
{
    TEST_SUITE("provisioning writer: golden vector and validation");
    RUN_TEST(test_golden_vector_parses);
    RUN_TEST(test_validation_accepts_golden_and_rejects_bounds);
    RUN_TEST(test_validation_rejects_record_corruption);
    RUN_TEST(test_validation_rejects_boot_and_adcs_out_of_range);
    RUN_TEST(test_validation_rejects_state_and_field_ranges);
    RUN_TEST(test_validation_rejects_ambiguous_equal_sequences);
    RUN_TEST(test_sequence_max_fixture_is_provisionable);
    RUN_TEST(test_rejected_image_touches_nothing);

    TEST_SUITE("provisioning writer: publication order and content");
    RUN_TEST(test_success_publishes_exact_golden_image);
    RUN_TEST(test_write_order_invalidates_markers_first_and_commits_last);
    RUN_TEST(test_provisioning_replaces_existing_records);

    TEST_SUITE("provisioning writer: interrupted-update recovery");
    RUN_TEST(test_every_stage_failure_never_publishes_a_partial_record);
    RUN_TEST(test_failure_reasons_are_reported);
    RUN_TEST(test_old_record_survives_interrupted_invalidation);

    TEST_RESULTS();
}
