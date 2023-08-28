#ifndef MOCKUP_FLASH_FLASH_H_
#define MOCKUP_FLASH_FLASH_H_

#include <stdint.h>
#include "../event_logger.h"
#include "../experiment_logger.h"

// Example of what the metadata could look like
// This could be read once before logging and then the logger can use it to track logging.
struct FlashLogMetadata
{
  uint64_t buffer_addr;   // Holds address of logs in flash
  uint64_t tail;          // location of next position to add log. Can be absolute addr or offset from buffer_addr (i.e. 0 means at buffer addr)
  uint64_t buffer_size;   // Can use size of buffer or last address to insert at.
  uint64_t log_size;      // Holds the size of a single log (i.e. how many uint64_ts). Useful for if we want this to be more general. Otherwise we can hardcode it.
};

// A generic function to pull metadata from the log section headers
// Meant to be called before you write the first log
// The most important part is the tail element as it WILL change as the program runs and must persist
// The rest technically just stay the same and could be hard coded.
struct FlashLogMetadata get_log_metadata(uint64_t flash_log_buffer[]);

// EVENT LOGS
// Copies event logs in bulk from local storage to mock flash
// "empties" local buffer by setting the tail of of the local logs struct to 0
// #define MOCK_EVENT_LOG_BUFFER_SIZE 2 + (1 * 468750)
#define MOCK_EVENT_LOG_BUFFER_SIZE 2 + (1 * 64)
uint8_t push_event_logs_to_flash(struct LocalEventLogs * local_event_logs);
extern uint64_t flash_event_log_buffer[MOCK_EVENT_LOG_BUFFER_SIZE];
extern struct FlashLogMetadata flash_event_logs_metadata;

// Copies exp logs in bulk from local storage to mock flash
// "empties" local buffer by setting tail to 0
uint8_t push_local_exp_to_flash(struct LocalExpLogs * local_exp_logs);
// #define MOCK_EXPERIMENT_LOG_BUFFER_SIZE 2 + (2 * 78125)
#define MOCK_EXPERIMENT_LOG_BUFFER_SIZE 2 + (2 * 64)
extern uint64_t flash_exp_log_buffer[MOCK_EXPERIMENT_LOG_BUFFER_SIZE];
extern struct FlashLogMetadata flash_exp_logs_metadata;












#endif