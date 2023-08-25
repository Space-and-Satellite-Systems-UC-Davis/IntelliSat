#include "experiment_logger.h"
#include "mockup_flash/flash.h"
#include "fits_in_bits.h"

#include <stdint.h>
#include <stdio.h>

// Small buffer that represents log storage on the MCU
// #define LOCAL_EXP_LOG_BUFFER_SIZE (2 * 32)



struct LocalExpLogs local_exp_logs = {
    .buffer_size = LOCAL_EXP_LOG_BUFFER_SIZE,
    .index_to_insert_at = 0,
    .buffer = {0}
};

int is_exp_being_logged();

void start_exp_logging();

void stop_exp_logging(int experiment_status);

void handle_exp_overflow();

void add_exp_log( union ExperimentLog* exp_log, struct LocalExpLogs * local_exp_logs ) {
	uint64_t addr_to_insert_at = local_exp_logs->index_to_insert_at;

	// Current overflow policy - Just overwrite oldest log
	// TODO: Put into separate function - detect_exp_buff_overflow()
	// Simplest is probably just pass the buffer and have it return next index to insert at?

	local_exp_logs->buffer[addr_to_insert_at] = exp_log->as_arr[0];
	local_exp_logs->buffer[addr_to_insert_at + 1] = exp_log->as_arr[1];

    local_exp_logs->index_to_insert_at = (addr_to_insert_at + 2) % local_exp_logs->buffer_size;
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

    add_exp_log(&exp_log, local_exp_logs);
    return 0;
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
    for (int i = 0; i < 128; ++i) {
        build_and_add_exp_log(i, i, i, i, i, i, i, i, &local_exp_logs);
    }

    for(int i = 0; i < local_exp_logs.buffer_size - 1; i += 2) {
        printf("Exp Log, rtc_time: %u\n", ((union ExperimentLog){local_exp_logs.buffer[i], local_exp_logs.buffer[i + 1]}).as_struct.rtc_time );
    }

    push_local_exp_to_flash(&local_exp_logs);
}