#include "event_logger.h"
#include "fits_in_bits.h"

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#define LOCAL_EVENT_LOG_BUFFER_SIZE 2 + (1 * 64)


// Small buffer that represents log storage on the MCU
uint64_t local_event_log_buffer[LOCAL_EVENT_LOG_BUFFER_SIZE] = {[0] = LOCAL_EVENT_LOG_BUFFER_SIZE, [1] = 1};

uint8_t get_local_event_log(uint64_t idx, uint64_t const event_log_buff[], union EventLog * const retrieved_log) {
    if(idx < 2 || idx >= event_log_buff[0]) {
        return -1;
    } else {
        retrieved_log->as_uint64 = event_log_buff[idx];
        return 0;
    }
}

uint8_t get_latest_event_log(uint64_t const event_log_buff[], union EventLog * const retrieved_log) {
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
        printf("Improper value\n");
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

// Some testing of overflows and such
// int main() {
//     for (int i = 0; i < 128; ++i) {
//         build_and_add_event_log(i, 0, 0, i, i, local_event_log_buffer);
//     }

//     int last_inserted_idx = local_event_log_buffer[1];
//     for(int i = 2; i < last_inserted_idx; ++i) {
//         printf("Event Log, rtc_time: %u\n", ((union EventLog)local_event_log_buffer[i]).as_struct.rtc_datetime);
//     }
// }