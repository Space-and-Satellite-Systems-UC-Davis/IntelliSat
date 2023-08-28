#include "experiment_logger.h"
#include "mockup_flash/flash.h"
#include "fits_in_bits.h"

#include <stdint.h>
#include <stdio.h>

// Small buffer that represents log storage on the MCU
// #define LOCAL_EXP_LOG_BUFFER_SIZE (2 * 32)

struct LocalExpLogs local_exp_logs = {
    .buffer_size = LOCAL_EXP_LOG_BUFFER_SIZE,
    .tail = 0,
    .buffer = {0}
};

int is_exp_being_logged();

void start_exp_logging();

void stop_exp_logging(int experiment_status);

void handle_exp_overflow();


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
    unsigned int extra,
    struct LocalExpLogs * local_exp_logs
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

    add_exp_log(exp_log.as_arr, local_exp_logs);
    return 0;
}

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

// Some testing of overflows and such
int main() {

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
    
    unsigned int curr_idx = 0;
    for (int i = 0; i < 128; ++i) {
        // build_and_add_exp_log(i, i, i, i, i, i, i, i, &local_exp_logs);
        build_exp_2(i, i, i, i, i, i, i, i, &local_exp_logs);
        
        if (local_exp_logs.tail == 0) {
            // printf("Moving from local to flash\n");
            push_local_exp_to_flash(&local_exp_logs);
        }
    }

    struct UnpackedExpLog u_exp;
    printf("Local Logs:\n");
    for(int i = 0; i < local_exp_logs.buffer_size - 1; i += 2) {
        u_exp = decode(local_exp_logs.buffer[i], local_exp_logs.buffer[i + 1]);
        // printf("L Exp Log, rtc_time: %u\n", ((union ExperimentLog){local_exp_logs.buffer[i], local_exp_logs.buffer[i + 1]}).as_struct.rtc_time );
        printf("L Exp Log, rtc_time: %u, gyro_x: %i\n", u_exp.rtc_time, u_exp.gyro_x );
    }
    
    // printf("\nMock Flash Logs:\n");
    // for(int i = 2; i < flash_exp_logs_metadata.buffer_size - 1; i += 2) {
    //     u_exp = decode(flash_exp_log_buffer[i], flash_exp_log_buffer[i + 1]);
    //     printf("F Exp Log, rtc_time: %u, gyro_x: %u\n", exp_log.as_struct.rtc_time, exp_log.as_struct.gyro_x );
    // }
}