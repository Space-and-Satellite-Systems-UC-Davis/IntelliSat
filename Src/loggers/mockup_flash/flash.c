#include "flash.h"
#include "../experiment_logger.h"
#include "../event_logger.h"
#include <stdint.h>

// EVENT LOGGING
#define EVENT_LOG_BUFFER_SIZE 2 + (1 * 468750)
uint64_t flash_event_log_buffer[EVENT_LOG_BUFFER_SIZE] = {[0] = EVENT_LOG_BUFFER_SIZE, [1] = 1};

uint8_t push_event_logs_to_flash(uint64_t local_buff[]) {
    uint64_t target_buff_size = local_buff[0];
    uint64_t buff_to_push_size = flash_event_log_buffer[0];

    if( target_buff_size < buff_to_push_size ) {
        return 1;
    }

    uint64_t target_latest_event_idx = local_buff[1];
    uint64_t buff_to_push_latest_event_idx = flash_event_log_buffer[1];

    for(uint64_t i = 2; i <= buff_to_push_latest_event_idx; i++) {
        target_latest_event_idx++;
        if (target_latest_event_idx >= target_buff_size) {
            target_latest_event_idx = 2;
        }
        local_buff[target_latest_event_idx] = flash_event_log_buffer[i];
    }

    // Currently we "clear" the local buffer
    // Works if we push the buffers contents once it fills up
    flash_event_log_buffer[1] = 1;
    local_buff[1] = target_latest_event_idx;
    return 0;
}


// EXPERIMENT LOGGING
// Buffer that represents the flash
#define EXPERIMENT_LOG_BUFFER_SIZE 2 + (2 * 78125)
uint64_t flash_exp_log_buffer[EXPERIMENT_LOG_BUFFER_SIZE] = {[0] = EXPERIMENT_LOG_BUFFER_SIZE, [1] = 0};

uint8_t push_local_exp_to_flash(struct LocalExpLogs * local_exp_logs) {
    uint64_t flash_buff_end_addr = flash_exp_log_buffer[0];
    uint64_t local_buff_size = local_exp_logs->buffer_size;

    if( flash_buff_end_addr < local_buff_size ) {
        // printf("target_buff < buff_to_push");
        return 1;
    }

    uint64_t flash_latest_exp_idx = flash_exp_log_buffer[1];
    // uint64_t local_latest_exp_idx = local_exp_logs->index_to_insert_at;

    for(uint64_t i = 2; i < local_buff_size; i += 2) {
        flash_latest_exp_idx += 2;
        if (flash_latest_exp_idx >= flash_buff_end_addr) {
            flash_latest_exp_idx = 2;
        }
        // printf("Pushing (%lu, %lu) -> (%lu, %lu)\n", i, i + 1, target_latest_exp_idx, target_latest_exp_idx + 1 );
        flash_exp_log_buffer[flash_latest_exp_idx] = local_exp_logs->buffer[i];
        flash_exp_log_buffer[flash_latest_exp_idx + 1] = local_exp_logs->buffer[i + 1];

    }

    // Could do this on local buffer overflows
    local_exp_logs->index_to_insert_at = 0;
    flash_exp_log_buffer[1] = flash_latest_exp_idx;
    return 0;
}