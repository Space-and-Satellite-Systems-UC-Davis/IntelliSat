#include <stdint.h>
#include <stdio.h>

int is_logging;				// Boolean
int _exp_header_address;	// in FLASH
int _start_address;			// in FLASH
int _end_address;			// in FLASH
int _curr_address;			// in FLASH

#define EXPERIMENT_LOG_BUFFER_SIZE 2 + (2 * 78125)
#define LOCAL_EXP_LOG_BUFFER_SIZE 2 + (2 * 32)

union ExperimentLog
{
// 128 bits == 16 bytes
    struct __attribute__((packed))
    {
        unsigned int rtc_time: 12;
        int16_t gyro_x, gyro_y, gyro_z;
        int16_t dgyro_x, dgyro_y, dgyro_z;
        unsigned int parity: 11;
    } as_struct;
    uint64_t as_arr[2];
};

// Buffer that represents the flash
uint64_t flash_exp_log_buffer[EXPERIMENT_LOG_BUFFER_SIZE] = {[0] = EXPERIMENT_LOG_BUFFER_SIZE, [1] = 0};

// Small buffer that represents log storage on the MCU
uint64_t local_exp_log_buffer[LOCAL_EXP_LOG_BUFFER_SIZE] = {[0] = LOCAL_EXP_LOG_BUFFER_SIZE, [1] = 0};

int is_exp_being_logged();

void start_exp_logging();

void stop_exp_logging(int experiment_status);

void handle_exp_overflow();

void add_exp_log( union ExperimentLog* exp_log, uint64_t exp_log_buff[] ) {
	uint64_t addr_to_insert_at = exp_log_buff[1] + 2;

	// Current overflow policy - Just overwrite oldest log
	// TODO: Put into separate function - detect_exp_buff_overflow()
	// Simplest is probably just pass the buffer and have it return next index to insert at?
	if (addr_to_insert_at >= exp_log_buff[0]) {
		addr_to_insert_at = 2;
	}

	exp_log_buff[1] = addr_to_insert_at;

	exp_log_buff[addr_to_insert_at] = exp_log->as_arr[0];
	exp_log_buff[addr_to_insert_at + 1] = exp_log->as_arr[1];
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
    unsigned int parity,
    uint64_t exp_log_buff[]
) {

    if (   !fits_in_bits(rtc_time, 12)
        || !fits_in_bits(parity, 11)
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
        .parity = parity
    }};

    add_exp_log(&exp_log, exp_log_buff);
    return 0;
}


uint8_t push_local_to_flash(uint64_t local_buff[], uint64_t flash_buff[]) {
    uint64_t flash_buff_end_addr = flash_buff[0];
    uint64_t local_buff_end_addr = local_buff[0];

    if( flash_buff_end_addr < local_buff_end_addr ) {
        // printf("target_buff < buff_to_push");
        return 1;
    }

    uint64_t flash_latest_exp_idx = flash_buff[1];
    uint64_t local_latest_exp_idx = local_buff[1];

    for(uint64_t i = 2; i <= local_latest_exp_idx; i += 2) {
        flash_latest_exp_idx += 2;
        if (flash_latest_exp_idx >= flash_buff_end_addr) {
            flash_latest_exp_idx = 2;
        }
        // printf("Pushing (%lu, %lu) -> (%lu, %lu)\n", i, i + 1, target_latest_exp_idx, target_latest_exp_idx + 1 );
        flash_buff[flash_latest_exp_idx] = local_buff[i];
        flash_buff[flash_latest_exp_idx + 1] = local_buff[i + 1];

    }

    // "clears" the local buffer
    // Could do this on local buffer overflows
    local_buff[1] = 0;
    flash_buff[1] = flash_latest_exp_idx;
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
