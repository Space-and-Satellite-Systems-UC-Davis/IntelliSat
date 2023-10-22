#include "event_logger.h"
#include "mockup_flash/flash.h"
#include "fits_in_bits.h"

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>


struct LocalEventLogs local_event_logs = {
    .buffer_size = LOCAL_EVENT_LOG_BUFF_SIZE,
    .tail = 0,
    .buffer = {0}
};

// TODO: log overflows
void handle_event_overflow() {
    push_event_logs_to_flash(&local_event_logs);
}

uint8_t add_event_log(
    uint64_t event_log
) {
    uint64_t current_log_index = local_event_logs.tail;

    local_event_logs.buffer[current_log_index] = event_log;
    
    current_log_index++;
    
    if (current_log_index >= local_event_logs.buffer_size) {
        printf("overflow\n");
        local_event_logs.tail = 0;
        handle_event_overflow();
    } else {
        local_event_logs.tail = current_log_index;
    }

    return 0;
}

uint8_t build_and_add_event_log(
    unsigned int rtc_datetime,     // Date+Hr+Min+Sec
    unsigned int current_mode,
    unsigned int action,
    unsigned int details,
    unsigned int extra
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

    add_event_log(event_log.as_uint64);
    return 0;
}

/*
int main() {

    init_flash_header();
    fetch_flash_header();

    for (int i = 0; i < 128; ++i) {
        build_and_add_event_log(i, 0, 0, 0, i);
    }

    printf("Local Logs:\n");
    union EventLog ev_log;
    for(uint32_t i = 0; i < local_event_logs.buffer_size; ++i) {
        ev_log = (union EventLog)local_event_logs.buffer[i];
        printf("Local ev Log #%u - rtc_time: %u, extra: %u\n", i, ev_log.as_struct.rtc_datetime, ev_log.as_struct.extra );
    }
    puts("A");

    uint64_t block_buff[32];
    uint32_t block_addr;

    enum LogType log_type = get_oldest_page(block_buff, &block_addr);
    assert(log_type == EVENT);

    union EventLog ev_logs[sizeof(block_buff)/sizeof(union EventLog)];

    static_assert(sizeof(ev_logs) >= sizeof(block_buff), "Hmm");

    memcpy(ev_logs, block_buff, sizeof(ev_logs));
}
*/
