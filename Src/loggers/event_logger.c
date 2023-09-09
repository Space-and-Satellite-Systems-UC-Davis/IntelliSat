#include "event_logger.h"
#include "mockup_flash/flash.h"
#include "fits_in_bits.h"

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

// Small buffer that represents log storage on the MCU
uint64_t local_event_log_buffer[LOCAL_EVENT_LOG_BUFFER_SIZE] = {[0] = LOCAL_EVENT_LOG_BUFFER_SIZE, [1] = 1};

uint8_t add_event_log(
    uint64_t event_log,
    struct LocalEventLogs * local_event_logs
) {
    uint64_t current_log_index = local_event_logs->tail;

    local_event_logs->buffer[current_log_index] = event_log;
    
    current_log_index++;
    
    if (current_log_index >= local_event_logs->buffer_size) {
        local_event_logs->tail = 0;
    } else {
        local_event_logs->tail = current_log_index;
    }

    return 0;
}

struct LocalEventLogs local_event_logs = {
    .buffer_size = LOCAL_EVENT_LOG_BUFFER_SIZE,
    .tail = 0,
    .buffer = {0}
};

uint8_t build_and_add_event_log(
    unsigned int rtc_datetime,     // Date+Hr+Min+Sec
    unsigned int current_mode,
    unsigned int action,
    unsigned int details,
    unsigned int extra,
    struct LocalEventLogs * local_event_logs
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

    add_event_log(event_log.as_uint64, local_event_logs);
    return 0;
}


/*
int main() {
    flash_event_logs_metadata = get_log_metadata(flash_event_log_buffer);

    unsigned int curr_idx = 0;
    for (int i = 0; i < 128; ++i) {
        build_and_add_event_log(i, 0, 0, 0, i, &local_event_logs);
        if (local_event_logs.tail == 0) {
            printf("Moving from local to flash\n");
            push_event_logs_to_flash(&local_event_logs);
        }
    }
    
    printf("Local Logs:\n");
    for(int i = 0; i < local_event_logs.buffer_size; ++i) {
        printf("L Event Log, rtc_time: %u\n", ((union EventLog){local_event_logs.buffer[i]}).as_struct.rtc_datetime );
    }

    
    printf("\nMock Flash Logs:\n");
    for(int i = 2; i < flash_event_logs_metadata.buffer_size; ++i) {
        printf("F Event Log, rtc_time: %u\n", ((union EventLog){flash_event_log_buffer[i]}).as_struct.rtc_datetime );
    }
}
*/
