#include "event_logger.h"
#include "logger_to_flash.h"
#include "fits_in_bits.h"

#include "downlink_logs.h"

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

struct LocalEventLogs local_event_logs = {
    .num_logs = LOCAL_EVENT_LOG_COUNT,
    .tail = 0,
    .logs = {{0, 0, 0, 0, 0}}
};

// TODO: log overflows
void handleEventOverflow() {
//    LOGGER_push_event_logs_to_flash();
	logger_downlinkEventLogs();
}

uint8_t buildAddEventLog(
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
        // printf("Improper value\n");
        return 1;
    }

    uint64_t current_log_index = local_event_logs.tail;

    local_event_logs.logs[current_log_index].rtc_datetime = rtc_datetime,
    local_event_logs.logs[current_log_index].current_mode = current_mode;
    local_event_logs.logs[current_log_index].action = action;
    local_event_logs.logs[current_log_index].details = details;
    local_event_logs.logs[current_log_index].extra = extra;

    current_log_index++;

    if( current_log_index >= local_event_logs.num_logs ) {
        local_event_logs.tail = 0;
        handleEventOverflow();

    } else {
        local_event_logs.tail = current_log_index;
    }
    return 0;
}
