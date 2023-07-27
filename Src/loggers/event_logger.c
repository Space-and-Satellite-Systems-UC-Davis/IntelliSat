#include "event_logger.h"
#include "fits_in_bits.h"

#include <stdio.h>
#include <stdint.h>
#include <assert.h>


#define EVENT_LOG_BUFFER_SIZE 2 + (1 * 468750)
#define LOCAL_EVENT_LOG_BUFFER_SIZE 2 + (1 * 64)

// Buffer that represents the NOR flash
uint64_t flash_event_log_buffer[EVENT_LOG_BUFFER_SIZE] = {[0] = EVENT_LOG_BUFFER_SIZE, [1] = 1};

// Small buffer that represents log storage on the MCU
uint64_t local_event_log_buffer[LOCAL_EVENT_LOG_BUFFER_SIZE] = {[0] = EVENT_LOG_BUFFER_SIZE, [1] = 1};

uint8_t get_local_event_log(uint64_t idx, uint64_t const event_log_buff[], union EventLog * const retrieved_log) {
    if(idx < 2 || idx >= event_log_buff[0]) {
        return -1;
    } else {
        retrieved_log->as_uint64 = event_log_buff[idx];
        return 0;
    }
}

int8_t get_latest_event_log(uint64_t const event_log_buff[], union EventLog * const retrieved_log) {
    retrieved_log->as_uint64 = event_log_buff[event_log_buff[1]];
    return 0;
}

uint8_t add_event_log(
    union EventLog* event_log,
    uint64_t event_log_buff[]
) {
    uint64_t current_log_index = event_log_buff[1] + 1;

    if (current_log_index >= event_log_buff[0]) {
        current_log_index = 2;
    }

    event_log_buff[1] = current_log_index;

    event_log_buff[current_log_index] = event_log->as_uint64;
    return 0;
}

uint8_t build_and_add_event_log(
    unsigned int rtc_datetime,     // Date+Hr+Min+Sec
    unsigned int current_mode,
    unsigned int action,
    unsigned int details,
    unsigned int extra,
    uint64_t event_log_buff[]
) {

    if (   !fits_in_bits(rtc_datetime, 22)
        || !fits_in_bits(current_mode, 4)
        || !fits_in_bits(action, 3)
        || !fits_in_bits(details, 24)
        || !fits_in_bits(extra, 11)
    ) {
        // Gave too large a value somewhere :(
        return 1;
    }

    union EventLog event_log =  {.as_struct = {
        .rtc_datetime = rtc_datetime,
        .current_mode = current_mode,
        .action = action,
        .details = details,
        .extra = extra
    }};

    add_event_log(&event_log, event_log_buff);
    return 0;
}

uint8_t push_local_to_flash(uint64_t local_buff[], uint64_t flash_buff[]) {
    uint64_t target_buff_size = local_buff[0];
    uint64_t buff_to_push_size = flash_buff[0];

    if( target_buff_size < buff_to_push_size ) {
        return 1;
    }

    uint64_t target_latest_event_idx = local_buff[1];
    uint64_t buff_to_push_latest_event_idx = flash_buff[1];

    for(uint64_t i = 2; i <= buff_to_push_latest_event_idx; i++) {
        target_latest_event_idx++;
        if (target_latest_event_idx >= target_buff_size) {
            target_latest_event_idx = 2;
        }
        local_buff[target_latest_event_idx] = flash_buff[i];
    }

    // Currently we "clear" the local buffer
    // Works if we push the buffers contents once it fills up
    flash_buff[1] = 1;
    local_buff[1] = target_latest_event_idx;
    return 0;
}




