/*
 * fram_device_fake.c
 *
 * The memory FRAM device used by the host tests. It provides the four checked
 * low-level driver entry points that startup_fram_provision.c calls; keeping the
 * signatures from Src/peripherals/FRAM/MB85RS256B.h means a device API change
 * breaks this build instead of being hidden by a permissive stub.
 */

#include "fram_device_fake.h"

#include "MB85RS256B.h"

typedef struct {
    uint16_t address;
    uint16_t length;
} FakeWriteOp;

static uint8_t device[FAKE_FRAM_SIZE];
static FakeWriteOp write_log[FAKE_FRAM_MAX_WRITE_OPS];
static unsigned write_ops;
static unsigned read_ops;
static bool out_of_range;
static bool write_enabled;

static FakeFramFault fault_kind;
static unsigned fault_op;
static unsigned fault_detail;
static unsigned pending_read_failures;
static bool pending_wrdi_failure;
static unsigned write_enable_fail_op = UINT16_MAX;

void fake_fram_reset(void)
{
    fake_fram_fill(0xFFu);
    write_ops = 0u;
    read_ops = 0u;
    out_of_range = false;
    write_enabled = false;
    fault_kind = FAKE_FAULT_NONE;
    fault_op = 0u;
    fault_detail = 0u;
    pending_read_failures = 0u;
    pending_wrdi_failure = false;
    write_enable_fail_op = UINT16_MAX;
}

void fake_fram_fill(uint8_t value)
{
    for (unsigned index = 0; index < FAKE_FRAM_SIZE; index++) {
        device[index] = value;
    }
}

void fake_fram_seed(uint16_t address, const uint8_t *data, uint16_t length)
{
    for (uint16_t index = 0; index < length; index++) {
        if (address + index < FAKE_FRAM_SIZE) {
            device[address + index] = data[index];
        }
    }
}

const uint8_t *fake_fram_memory(void)
{
    return device;
}

uint8_t fake_fram_byte(uint16_t address)
{
    return address < FAKE_FRAM_SIZE ? device[address] : 0u;
}

void fake_fram_fail_write(unsigned op_index, FakeFramFault fault, unsigned detail)
{
    fault_op = op_index;
    fault_kind = fault;
    fault_detail = detail;
}

void fake_fram_fail_write_enable(unsigned op_index)
{
    write_enable_fail_op = op_index;
}

unsigned fake_fram_write_ops(void)
{
    return write_ops;
}

uint16_t fake_fram_write_address(unsigned op_index)
{
    return op_index < write_ops ? write_log[op_index].address : UINT16_MAX;
}

uint16_t fake_fram_write_length(unsigned op_index)
{
    return op_index < write_ops ? write_log[op_index].length : 0u;
}

unsigned fake_fram_read_ops(void)
{
    return read_ops;
}

bool fake_fram_out_of_range(void)
{
    return out_of_range;
}

/* ---- the low-level driver surface ---- */

bool FRAM_writeEnable(void)
{
    if (write_ops == write_enable_fail_op) {
        write_enabled = false;
        return false;
    }
    write_enabled = true;
    return true;
}

bool FRAM_writeDisable(void)
{
    write_enabled = false;
    if (pending_wrdi_failure) {
        pending_wrdi_failure = false;
        return false;
    }
    return true;
}

uint8_t FRAM_readStatusRegister(void)
{
    return write_enabled ? 0x02u : 0x00u;
}

bool FRAM_readData(uint16_t address, uint8_t *buffer, uint16_t size)
{
    if (buffer == NULL || size == 0u || address >= FAKE_FRAM_SIZE
        || size > (uint16_t)(FAKE_FRAM_SIZE - address)) {
        out_of_range = true;
        return false;
    }
    read_ops++;
    if (pending_read_failures > 0u) {
        pending_read_failures--;
        return false;
    }
    for (uint16_t index = 0; index < size; index++) {
        buffer[index] = device[address + index];
    }
    return true;
}

bool FRAM_writeData(uint16_t address, uint8_t *data, uint16_t size)
{
    const unsigned op_index = write_ops;
    if (data == NULL || size == 0u || address >= FAKE_FRAM_SIZE
        || size > (uint16_t)(FAKE_FRAM_SIZE - address)) {
        out_of_range = true;
        return false;
    }
    if (op_index < FAKE_FRAM_MAX_WRITE_OPS) {
        write_log[op_index].address = address;
        write_log[op_index].length = size;
    }
    write_ops++;

    const FakeFramFault fault =
        (fault_kind != FAKE_FAULT_NONE && op_index == fault_op) ? fault_kind
                                                                : FAKE_FAULT_NONE;
    uint16_t applied = size;
    switch (fault) {
    case FAKE_FAULT_HARD_ERROR:
        return false;
    case FAKE_FAULT_SHORT_WRITE:
        applied = (uint16_t)(fault_detail < size ? fault_detail : size);
        break;
    case FAKE_FAULT_READ_ERROR:
        pending_read_failures = fault_detail > 0u ? fault_detail : 1u;
        break;
    case FAKE_FAULT_WRDI_ERROR:
        pending_wrdi_failure = true;
        break;
    default:
        break;
    }

    for (uint16_t index = 0; index < applied; index++) {
        device[address + index] = data[index];
    }
    if (fault == FAKE_FAULT_CORRUPT_BYTE) {
        const uint16_t target =
            (uint16_t)(address + (fault_detail < applied ? fault_detail : 0u));
        device[target] ^= 0x01u;
    }
    if (fault == FAKE_FAULT_LOST_RESPONSE) {
        return false;
    }
    return true;
}
