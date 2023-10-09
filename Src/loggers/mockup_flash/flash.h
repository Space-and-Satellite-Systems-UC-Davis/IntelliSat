#ifndef MOCKUP_FLASH_FLASH_H_
#define MOCKUP_FLASH_FLASH_H_

#include <stdint.h>
#include "../event_logger.h"
#include "../experiment_logger.h"

#pragma pack(push,1)
struct EventLogHeader
{
  uint32_t start_addr: 24;
  uint32_t end_addr: 24;
  uint32_t latest_addr: 24;
  uint32_t oldest_block_addr: 24;
};

struct ExperimentLogHeader
{
  uint32_t start_addr: 24;
  uint32_t end_addr: 24;
  uint32_t latest_addr: 24;
  uint32_t oldest_block_addr: 24;

  uint32_t start_datetime: 22;
  uint8_t exit_status: 4;
  uint16_t extra: 14;
};

struct FlashHeader
{
  struct EventLogHeader events_header;
  struct ExperimentLogHeader exp1_header;
  struct ExperimentLogHeader exp2_header;
  struct ExperimentLogHeader exp3_header;
  struct ExperimentLogHeader exp4_header;
  struct ExperimentLogHeader exp5_header;
  uint8_t current_exp_num: 8;
  uint32_t backup_tle_addr: 24;
  uint8_t extra[155];
};
#pragma pack(pop)

// EVENT LOGS
// Copies event logs in bulk from local storage to mock flash
// "empties" local buffer by setting the tail of of the local logs struct to 0
uint8_t push_event_logs_to_flash(struct LocalEventLogs * local_event_logs);

// #define MOCK_EVENT_LOG_BUFFER_SIZE 468750
#define MOCK_EVENT_LOG_COUNT 64


// Copies exp logs in bulk from local storage to mock flash
// "empties" local buffer by setting tail to 0
uint8_t push_exp_logs_to_flash(struct LocalExpLogs * local_exp_logs,  struct ExperimentLogHeader * exp_header);

// #define MOCK_EXPERIMENT_LOG_BUFFER_SIZE 78125
#define MOCK_EXP_LOG_COUNT 64


// In-memory copy of the header stored in the (mock) flash
extern struct FlashHeader flash_header;

// Copies flash header data from mock flash to flash header struct
// Can be used on startups after the first one, or perhaps for ECC tasks
// void get_flash_header(struct FlashHeader * out);
void fetch_flash_header();


// Initialize header in flash with some default values. 
// Useful for setting up the flash for the first time.
void init_flash_header();

enum LogType
{
  EVENT = 0,
  EXP1 = 1,
  EXP2 = 2,
  EXP3 = 3,
  EXP4 = 4,
  EXP5 = 5
};


// Updates the header in mock flash by copying struct to header 
void update_flash_header();


// copies the oldest page from flash as `page_buff` argument
// Returns the type of log (for ECC subsystem to inform flash driver where to write corrected page)
enum LogType get_oldest_page(uint64_t page_buff[32], uint32_t * block_addr);



#endif