#include "experiment_logger.h"
#include "mockup_flash/flash.h"
#include "fits_in_bits.h"

#include <stdint.h>
#include <stdio.h>

// Small buffer that represents log storage on the MCU
// #define LOCAL_EXP_LOG_BUFFER_SIZE (2 * 32)

// TODO: turn this into a pointer to one of the two local logging structs
// Use this pointer when needed
// swap what it points to on local log overflow
struct LocalExpLogs local_exp_logs = {
    .buffer_size = LOCAL_EXP_LOG_BUFFER_SIZE,
    .tail = 0,
    .buffer = {0}
};

// static struct LocalExpLogs local_exp_logs_1 = {
//     .buffer_size = LOCAL_EXP_LOG_BUFFER_SIZE,
//     .tail = 0,
//     .buffer = {0}
// };

// static struct LocalExpLogs local_exp_logs_2 = {
//     .buffer_size = LOCAL_EXP_LOG_BUFFER_SIZE,
//     .tail = 0,
//     .buffer = {0}
// };

int is_exp_being_logged();

struct ExperimentLogHeader * current_exp_header;

// TODO: figure out what to set up when starting logging
void start_exp_logging() {
    
}

// TODO: Save experiment status to flash header
void stop_exp_logging(int experiment_status) {

    current_exp_header->exit_status = experiment_status;
    int current_exp = flash_header.current_exp_num + 1;

    if(current_exp > 5) {
        current_exp = 1;
    }

    switch(current_exp) {
        case 1: {
            current_exp_header = &flash_header.exp1_header;
            break;
        }
        case 2: {
            current_exp_header = &flash_header.exp2_header;
            break;
        }
        case 3: {
            current_exp_header = &flash_header.exp3_header;
            break;
        }
        case 4: {
            current_exp_header = &flash_header.exp4_header;
            break;
        }
        case 5: {
            current_exp_header = &flash_header.exp5_header;
            break;
        }
    }
    
    flash_header.current_exp_num = current_exp;
    update_flash_header();
}

struct LocalEventLogs * loc_test;

// TODO: swap out the local buffer used for local logging on overflow
void handle_exp_overflow() {
    push_exp_logs_to_flash(&local_exp_logs, current_exp_header);

    // if(local_exp_logs == &local_exp_logs_1) {
    //     local_exp_logs = local_exp_logs_2;
    // } else {
    //     local_exp_logs = local_exp_logs_1;
    // }
}

void add_exp_log( uint64_t exp_log[2], struct LocalExpLogs * local_exp_logs ) {
	uint64_t current_log_index = local_exp_logs->tail;

	// Current overflow policy - Just overwrite oldest log
	// TODO: Put into separate function - detect_exp_buff_overflow()
	// Simplest is probably just pass the buffer and have it return next index to insert at?

	local_exp_logs->buffer[current_log_index] = exp_log[0];
	local_exp_logs->buffer[current_log_index + 1] = exp_log[1];

    current_log_index += 2;
    
    if ( (current_log_index + 1) > local_exp_logs->buffer_size ) {
        local_exp_logs->tail = 0;

        // TODO: Can place overflow handler here. Suggestion - have a handle
        handle_exp_overflow();
    } else {
        local_exp_logs->tail = current_log_index;
    }

    // local_exp_logs->index_to_insert_at = (addr_to_insert_at + 2);

	// printf("Inserting at idx: (%lu, %lu)\n", current_log_index, current_log_index + 1);
};

/**
 * Builds an experiment log from its arguments and passes them to add_exp_log()
 * Checks that the fields fit in the bitfields
 * Might just be useful for testing
 */
uint8_t build_and_add_exp_log(
    unsigned int rtc_time,     // Date+Hr+Min+Sec
    int16_t  gyro_x,
    int16_t  gyro_y,
    int16_t  gyro_z,
    int16_t  dgyro_x,
    int16_t  dgyro_y,
    int16_t  dgyro_z,
    unsigned int extra
) {

    if (   !fits_in_bits(rtc_time, 12)
        || !fits_in_bits(extra, 11)
    ) {
        // Gave too large a value somewhere :(
        return 1;
    }

    union ExperimentLog exp_log =  {.as_struct = {
        .rtc_time = rtc_time,
        .gyro_x = gyro_x,
        .gyro_y = gyro_y,
        .gyro_z = gyro_z,
        .dgyro_x = dgyro_x,
        .dgyro_y = dgyro_y,
        .dgyro_z = dgyro_z,
        .extra = extra
    }};

    add_exp_log(exp_log.as_arr, &local_exp_logs);
    return 0;
}

// Function for adding an experiment log using bitwire operations. This may be the way we have to move forward
uint8_t build_exp_2(
    uint16_t rtc_time,     // Date+Hr+Min+Sec
    int16_t  gyro_x,
    int16_t  gyro_y,
    int16_t  gyro_z,
    int16_t  dgyro_x,
    int16_t  dgyro_y,
    int16_t  dgyro_z,
    uint32_t extra,
    struct LocalExpLogs * local_exp_logs
) {

    if (   !fits_in_bits(rtc_time, 12)
        || !fits_in_bits(extra, 20)
    ) {
        // Gave too large a value somewhere :(
        return 1;
    }

    uint64_t log[2] = {0, 0};

    log[0] |= ((uint64_t)rtc_time << 52);
    log[0] |= ((uint64_t)gyro_x << 36);
    log[0] |= ((uint64_t)gyro_y << 20);
    log[0] |= ((uint64_t)gyro_z << 4);
    log[0] |= ((uint64_t)dgyro_x >> 12);

    log[1] |= ((uint64_t)dgyro_y << 52);
    log[1] |= ((uint64_t)dgyro_z << 36);
    log[1] |= extra;


    add_exp_log(log, local_exp_logs);
    return 0;
}

// Turns a pair of uint64_t into a single experiment log with bitwise operations.
// Currently unused - using bitfields, unions, and memcpy.
struct UnpackedExpLog decode(uint64_t log_0, uint64_t log_1) {
    struct UnpackedExpLog unpacked = {
        .rtc_time = (log_0 >> 52) & MASK_12_BIT,
        .gyro_x   = (log_0 >> 36) & MASK_16_BIT,
        .gyro_y   = (log_0 >> 20) & MASK_16_BIT,
        .gyro_z   = (log_0 >> 4)  & MASK_16_BIT,
        .dgyro_x  = (log_0 & MASK_4_BIT) + ((log_1 >> 52) & MASK_12_BIT),
        .dgyro_y  = (log_1 >> 36) & MASK_16_BIT,
        .dgyro_z  = (log_1 >> 20) & MASK_16_BIT,
        .extra    = (log_1 & MASK_20_BIT)
    };

    return unpacked;
}

uint8_t get_exp_log(uint64_t addr, uint64_t const exp_log_buff[], union ExperimentLog * const retrieved_log) {
    if(addr < 2 || addr >= exp_log_buff[0] - 1 || addr % 2 != 0) {
        // printf("idx %lu fail\n", idx);
        return -1;
    } else {
        retrieved_log->as_arr[0] = exp_log_buff[addr];
        retrieved_log->as_arr[1] = exp_log_buff[addr + 1];
        return 0;
    }
}

int detect_exp_buff_overflow();

void handle_exp_buff_overflow();

/*
// Some testing of overflows and such
int branch_main() {

     // Start Logging Experiment
     //  - get metadata:
     //      - exp log size
     //      - exp log current insert idx
     //      - exp log start addr?
     //  - store metadata in struct
     //  - store logs locally
     //  - upon buffer fill:
     //      - move local logs to flash (could offload to DMA???)
     //      - After transfer is complete, update flash's metadata ( mainly log head )
     //      - possibly swap buffer used for local logging in the meantime

	// get_flash_header(&flash_header);



    unsigned int curr_idx = 0;
    for (int i = 0; i < 128; ++i) {
        build_and_add_exp_log(i, i, i, i, i, i, i, i);
        // if (local_exp_logs.tail == 0) {
        //     printf("Moving from local to flash\n");
        //     push_exp_logs_to_flash(&local_exp_logs, current_exp_header);
        // }
    }

    printf("Local Logs:\n");
    for(uint64_t i = 0; i < local_exp_logs.buffer_size - 1; i += 2) {
        printf("L Exp Log, rtc_time: %u\n", ((union ExperimentLog){ {local_exp_logs.buffer[i], local_exp_logs.buffer[i + 1]}}).as_struct.rtc_time );
    }
}
*/
