#include "flash.h"
#include "../experiment_logger.h"
#include "../event_logger.h"
#include <stdint.h>

#include <assert.h>


struct FlashLogMetadata get_log_metadata(uint64_t flash_log_buffer[]) {
    struct FlashLogMetadata flash_log_metadata = {
        .buffer_addr = 0,
        .buffer_size = flash_log_buffer[0],
        .tail = flash_log_buffer[1],
        .log_size = 2
    };
    
    return flash_log_metadata;
}

// EVENT LOGGING
uint64_t flash_event_log_buffer[MOCK_EVENT_LOG_BUFFER_SIZE] = {[0] = MOCK_EVENT_LOG_BUFFER_SIZE, [1] = 2};

struct FlashLogMetadata flash_event_logs_metadata;

uint8_t push_event_logs_to_flash(struct LocalEventLogs * local_event_logs) {

    if( flash_event_logs_metadata.buffer_size < local_event_logs->buffer_size ) {
        // printf("target_buff < buff_to_push");
        return 1;
    }

    // uint64_t local_latest_exp_idx = local_exp_logs->index_to_insert_at;

    for(uint64_t i = 0; i < local_event_logs->buffer_size; ++i) {
        
        // printf("Pushing (%lu, %lu) -> (%lu, %lu)\n", i, i + 1, target_latest_exp_idx, target_latest_exp_idx + 1 );
        flash_event_log_buffer[flash_event_logs_metadata.tail] = local_event_logs->buffer[i];
        
        ++flash_event_logs_metadata.tail;
        if (flash_event_logs_metadata.tail >= flash_event_logs_metadata.buffer_size) {
            flash_event_logs_metadata.tail = 2;
        }
    }

    // Could do this on local buffer overflows
    local_event_logs->tail = 0;
    flash_event_log_buffer[1] = flash_event_logs_metadata.tail;
    return 0;
}


// EXPERIMENT LOGGING
// Buffer that represents the flash
uint64_t flash_exp_log_buffer[MOCK_EXPERIMENT_LOG_BUFFER_SIZE] = {[0] = MOCK_EXPERIMENT_LOG_BUFFER_SIZE, [1] = 2};
struct FlashLogMetadata flash_exp_logs_metadata = {
    .buffer_size = MOCK_EXPERIMENT_LOG_BUFFER_SIZE,
    .tail = 2
};

uint8_t push_local_exp_to_flash(struct LocalExpLogs * local_exp_logs) {
    // uint64_t local_buff_size = local_exp_logs->buffer_size;
    
    assert(flash_exp_logs_metadata.buffer_size > local_exp_logs->buffer_size);
    // if( flash_exp_logs_metadata.buffer_size < local_exp_logs->buffer_size ) {
    //     return 1;
    // }

    // printf("Start: %lu, ", flash_exp_logs_metadata.tail);

    for(uint64_t i = 0; i < local_exp_logs->buffer_size - 1; i += 2) {
        flash_exp_log_buffer[ flash_exp_logs_metadata.tail ] = local_exp_logs->buffer[i];
        flash_exp_log_buffer[ flash_exp_logs_metadata.tail + 1 ] = local_exp_logs->buffer[i + 1];

        flash_exp_logs_metadata.tail += 2;

        if (flash_exp_logs_metadata.tail >= flash_exp_logs_metadata.buffer_size - 1) {
            flash_exp_logs_metadata.tail = 2;
        }

    }


    // Could do this on local buffer overflows
    local_exp_logs->tail = 0;
    flash_exp_log_buffer[1] = flash_event_logs_metadata.tail;
    // printf("End: %lu\n", flash_exp_logs_metadata.tail);
    return 0;
}