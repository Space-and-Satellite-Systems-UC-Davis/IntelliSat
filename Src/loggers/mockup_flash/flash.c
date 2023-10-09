#include "flash.h"
// #include "../experiment_logger.h"
// #include "../event_logger.h"

#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stddef.h>

#define MOCK_EVENT_LOG_BUFF_SIZE (MOCK_EVENT_LOG_COUNT * 8)
#define MOCK_EXP_LOG_BUFF_SIZE (MOCK_EXP_LOG_COUNT * 8 * 2)

#define EVENT_LOG_START sizeof(struct FlashHeader)

#define EXP_LOG_1_START EVENT_LOG_START + MOCK_EVENT_LOG_BUFF_SIZE
#define EXP_LOG_1_END EXP_LOG_1_START + MOCK_EXP_LOG_BUFF_SIZE

#define EXP_LOG_2_START EXP_LOG_1_START + MOCK_EXP_LOG_BUFF_SIZE
#define EXP_LOG_2_END EXP_LOG_2_START + MOCK_EXP_LOG_BUFF_SIZE

#define EXP_LOG_3_START EXP_LOG_2_START + MOCK_EXP_LOG_BUFF_SIZE
#define EXP_LOG_3_END EXP_LOG_3_START + MOCK_EXP_LOG_BUFF_SIZE

#define EXP_LOG_4_START EXP_LOG_3_START + MOCK_EXP_LOG_BUFF_SIZE
#define EXP_LOG_4_END EXP_LOG_4_START + MOCK_EXP_LOG_BUFF_SIZE

#define EXP_LOG_5_START EXP_LOG_4_START + MOCK_EXP_LOG_BUFF_SIZE
#define EXP_LOG_5_END EXP_LOG_5_START + MOCK_EXP_LOG_BUFF_SIZE

static_assert(EVENT_LOG_START % 256 == 0, "");


// How many bytes in a single block when retrieved for ECC
// Currently assumes one block for ECC = one page of FLASH
#define ECC_BLOCK_SIZE 256

// A global variable to be used as our mock flash
uint8_t mock_flash_buff[sizeof(struct FlashHeader) + (MOCK_EVENT_LOG_BUFF_SIZE) + (MOCK_EXP_LOG_BUFF_SIZE * 5)];

// A global struct representing our flash header
const struct FlashHeader default_flash_header = {
    .events_header = {
        .start_addr = EVENT_LOG_START,
        .end_addr = EVENT_LOG_START + MOCK_EVENT_LOG_BUFF_SIZE,
        .latest_addr = EVENT_LOG_START,
        .oldest_block_addr = EVENT_LOG_START
    },
    .exp1_header = {
        .start_addr = EXP_LOG_1_START,
        .end_addr = EXP_LOG_1_END,
        .latest_addr = EXP_LOG_1_START,
        .oldest_block_addr = EXP_LOG_1_START
    },
    .exp2_header = {
        .start_addr = EXP_LOG_2_START,
        .end_addr = EXP_LOG_2_END,
        .latest_addr = EXP_LOG_2_START,
        .oldest_block_addr = EXP_LOG_2_START
    },
    .exp3_header = {
        .start_addr = EXP_LOG_3_START,
        .end_addr = EXP_LOG_3_END,
        .latest_addr = EXP_LOG_3_START,
        .oldest_block_addr = EXP_LOG_3_START
    },
    .exp4_header = {
        .start_addr = EXP_LOG_4_START,
        .end_addr = EXP_LOG_4_END,
        .latest_addr = EXP_LOG_4_START,
        .oldest_block_addr = EXP_LOG_4_START
    },
    .exp5_header = {
        .start_addr = EXP_LOG_5_START,
        .end_addr = EXP_LOG_5_END,
        .latest_addr = EXP_LOG_5_START,
        .oldest_block_addr = EXP_LOG_5_START
    },
    .current_exp_num = 0
};

struct FlashHeader flash_header;

void init_flash_header() {
    memcpy(mock_flash_buff, &default_flash_header, sizeof(default_flash_header));
    memcpy(&flash_header, &default_flash_header, sizeof(default_flash_header));
}

// TODO: could write these to match flash interface by just writing two:
//  - arbitrary byte read - takes a single address
//  - 256 byte "page program" - takes buffer up to 256 bytes, and handles overflow on page and stuff
//      - Could have a bulk write function that queues larger transfers. Might not be worth if no real gains
// void write_to_flash(void * buff, uint8_t size, uint32_t addr) {
//     uint32_t offset_from_256 = (addr % 256);
//     uint32_t cutoff;
//     if(size < )
//     memcpy(mock_flash_buff + addr, buff, size)

// }

// void read_from_flash(void * buff, uint32_t size);

void fetch_flash_header() {
    memcpy(&flash_header, mock_flash_buff, sizeof(struct FlashHeader));
}

void update_flash_header() {
    memcpy(mock_flash_buff, &flash_header, sizeof(struct FlashHeader));
}

static uint32_t advance_addr(const uint32_t start, const uint32_t end, const uint32_t addr_to_advance, const uint32_t increment) {
    uint32_t offset_from_start = addr_to_advance - start;
    return start + (offset_from_start + increment) % (end - start);
}

static void advance_oldest_event_block() {
    flash_header.events_header.oldest_block_addr = advance_addr(
        flash_header.events_header.start_addr,
        flash_header.events_header.end_addr,
        flash_header.events_header.oldest_block_addr,
        ECC_BLOCK_SIZE
    );
}

static void advance_oldest_exp_block(struct ExperimentLogHeader * exp_log_header) {
    exp_log_header->oldest_block_addr = advance_addr(
        exp_log_header->start_addr,
        exp_log_header->end_addr,
        exp_log_header->oldest_block_addr,
        ECC_BLOCK_SIZE
    );
}

enum LogType get_oldest_page(uint64_t page_buff[32], uint32_t * block_addr) {
    static enum LogType oldest_block_type = EVENT;
    uint32_t oldest_block_addr;
    switch(oldest_block_type) {
        case EVENT: {
            oldest_block_addr = flash_header.events_header.oldest_block_addr;
            advance_oldest_event_block();
        } break;

        case EXP1: {
            oldest_block_addr = flash_header.exp1_header.oldest_block_addr;
            advance_oldest_exp_block(&flash_header.exp1_header);

        } break;

        case EXP2: {
            oldest_block_addr = flash_header.exp2_header.oldest_block_addr;
            advance_oldest_exp_block(&flash_header.exp2_header);
        } break;

        case EXP3: {
            oldest_block_addr = flash_header.exp3_header.oldest_block_addr;
            advance_oldest_exp_block(&flash_header.exp3_header);

        } break;

        case EXP4: {
            oldest_block_addr = flash_header.exp4_header.oldest_block_addr;
            advance_oldest_exp_block(&flash_header.exp4_header);

        } break;

        case EXP5: {
            oldest_block_addr = flash_header.exp5_header.oldest_block_addr;
            advance_oldest_exp_block(&flash_header.exp5_header);
        } break;
    }

    update_flash_header();

    memcpy(page_buff, mock_flash_buff + oldest_block_addr, sizeof(uint64_t[32]));
    // memcpy(page_buff, mock_flash_buff + oldest_block_addr, ECC_BLOCK_SIZE);

    enum LogType block_type = oldest_block_type;

    oldest_block_type++;
    if(oldest_block_type > EXP5) oldest_block_type = EVENT;

    *block_addr = oldest_block_addr;
    return block_type;
}

/**
If the local buffer is longer than remaining space in mock flash buffer, this will write too much.

Potential solutions:
    1. Make the size of block a multiple of local buffer size, and only transfer when local buffer is full
    2. Check if size of block being transfered > remaining space in buffer and wrap around to start.
*/
uint8_t push_event_logs_to_flash(struct LocalEventLogs * local_event_logs) {
    memcpy(mock_flash_buff + flash_header.events_header.latest_addr, local_event_logs->buffer, sizeof(local_event_logs->buffer));
    flash_header.events_header.latest_addr = advance_addr(
        flash_header.events_header.start_addr,
        flash_header.events_header.end_addr,
        flash_header.events_header.latest_addr,
        sizeof(local_event_logs->buffer)
    );
}

uint8_t push_exp_logs_to_flash(struct LocalExpLogs * local_exp_logs, struct ExperimentLogHeader * current_exp_header) {
    memcpy(mock_flash_buff + current_exp_header->latest_addr, local_exp_logs->buffer, sizeof(local_exp_logs->buffer));
    current_exp_header->latest_addr = advance_addr(
        current_exp_header->start_addr,
        current_exp_header->end_addr,
        current_exp_header->latest_addr,
        sizeof(local_exp_logs->buffer)
    );
}


// uint8_t push_exp_logs_to_flash(struct LocalExpLogs * local_exp_logs) {
//     // uint64_t local_buff_size = local_exp_logs->buffer_size;
    
//     // assert(flash_exp_logs_metadata.buffer_size > local_exp_logs->buffer_size);
//     // if( flash_exp_logs_metadata.buffer_size < local_exp_logs->buffer_size ) {
//     //     return 1;
//     // }

//     // printf("Start: %lu, ", flash_exp_logs_metadata.tail);

//     for(uint64_t i = 0; i < local_exp_logs->buffer_size - 1; i += 2) {
//         flash_exp_log_buffer[ flash_exp_logs_metadata.tail ] = local_exp_logs->buffer[i];
//         flash_exp_log_buffer[ flash_exp_logs_metadata.tail + 1 ] = local_exp_logs->buffer[i + 1];

//         flash_exp_logs_metadata.tail += 2;

//         if (flash_exp_logs_metadata.tail >= flash_exp_logs_metadata.buffer_size - 1) {
//             flash_exp_logs_metadata.tail = 2;
//         }

//     }

//     // Could do this on local buffer overflows
//     local_exp_logs->tail = 0;
//     flash_exp_log_buffer[1] = flash_event_logs_metadata.tail;
//     // printf("End: %lu\n", flash_exp_logs_metadata.tail);
//     return 0;
// }

void print_exp(uint8_t exp_log[sizeof(union ExperimentLog)]) {
    union ExperimentLog exp;
    memcpy(&exp, exp_log, sizeof(union ExperimentLog));

    printf("exp_log:\n\trtc_time: %u\n\t gx: %u,  gy: %u,  gz: %u\n\tdgx: %u, dgy: %u, dgz: %u\n\textra: %u",
        exp.as_struct.rtc_time,
        exp.as_struct.gyro_x,
        exp.as_struct.gyro_y,
        exp.as_struct.gyro_z,
        exp.as_struct.dgyro_x,
        exp.as_struct.dgyro_y,
        exp.as_struct.dgyro_z,
        exp.as_struct.extra
    );
}

// int main() {
//     // Check that all regions are on start of 256 byte boundaries
//     assert(default_flash_header.events_header.start_addr % 256 == 0);
//     assert(default_flash_header.exp1_header.start_addr % 256 == 0);
//     assert(default_flash_header.exp2_header.start_addr % 256 == 0);
//     assert(default_flash_header.exp3_header.start_addr % 256 == 0);
//     assert(default_flash_header.exp4_header.start_addr % 256 == 0);
//     assert(default_flash_header.exp5_header.start_addr % 256 == 0);

//     init_flash_header();
//     get_flash_header();

//     // Checking that flash header was properly initialized to default
//     assert(memcmp(&flash_header, &default_flash_header, sizeof(default_flash_header)) == 0);

//     uint64_t buff[ECC_BLOCK_SIZE/8];

//     // Should return the events in the correct sequence
//     assert(get_oldest_page(buff) == EVENT);
//     assert(get_oldest_page(buff) == EXP1);
//     assert(get_oldest_page(buff) == EXP2);
//     assert(get_oldest_page(buff) == EXP3);
//     assert(get_oldest_page(buff) == EXP4);
//     assert(get_oldest_page(buff) == EXP5);
//     // Should start over again from 
//     assert(get_oldest_page(buff) == EVENT);

//     // The mock buffer should match the flash header:
//     assert(memcmp(&flash_header, mock_flash_buff, sizeof(flash_header)) == 0);
    
//     // After calling get_oldest_page, the flash header should be different from the default
//     assert(memcmp(&flash_header, &default_flash_header, sizeof(default_flash_header)) != 0);

//     // The only change should be the oldest_block_addrs for each set of logs
//     struct FlashHeader new_header = default_flash_header;
//     // events header should have rolled back to start
//     // new_header.events_header.oldest_block_addr;
//     new_header.exp1_header.oldest_block_addr += ECC_BLOCK_SIZE;
//     new_header.exp2_header.oldest_block_addr += ECC_BLOCK_SIZE;
//     new_header.exp3_header.oldest_block_addr += ECC_BLOCK_SIZE;
//     new_header.exp4_header.oldest_block_addr += ECC_BLOCK_SIZE;
//     new_header.exp5_header.oldest_block_addr += ECC_BLOCK_SIZE;

//     // Check that the new header matches the 
//     assert(memcmp(&flash_header, &new_header, sizeof(flash_header)) == 0);
//     assert(memcmp(&new_header, mock_flash_buff, sizeof(new_header)) == 0);
// }