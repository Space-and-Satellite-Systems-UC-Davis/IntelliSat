#ifndef MOCKUP_FLASH_FLASH_H_
#define MOCKUP_FLASH_FLASH_H_


#include "event_logger.h"
#include "experiment_logger.h"

#include <stdint.h>
#include "../peripherals/FLASH/W25Q128JV.h"
#include "../tools/print_scan.h"


#define EVENT_SECTORS        2
#define EXP_SECTORS          2

#define EVENT_START_SECTOR   1

#define EXP1_START_SECTOR    EVENT_START_SECTOR + EVENT_SECTORS
#define EXP1_END_SECTOR      EXP1_START_SECTOR + EXP_SECTORS

#define EXP2_START_SECTOR    EXP1_START_SECTOR + EVENT_SECTORS
#define EXP2_END_SECTOR      EXP2_START_SECTOR + EXP_SECTORS


#pragma pack(push,1)
struct EventLogHeader
{
    uint32_t start_sector_num: 24;
    uint32_t end_sector_num: 24;
    uint32_t tail: 24;
    uint32_t oldest_sector_num: 24;
};

struct ExperimentLogHeader
{
    uint32_t start_sector_num: 24;
    uint32_t end_sector_num: 24;
    uint32_t tail: 24;
    uint32_t oldest_sector_num: 24;

    uint32_t start_datetime: 22;
    uint8_t exit_status: 4;
    uint16_t extra: 14;
};

struct FlashHeader
{
    struct EventLogHeader events_header;
    struct ExperimentLogHeader exp1_header;
    struct ExperimentLogHeader exp2_header;
    uint8_t current_exp_num: 8;
    uint32_t backup_tle_addr: 24;
    uint8_t extra[206];
};
#pragma pack(pop)

// EVENT LOGS
// Copies event logs in bulk from local storage to mock flash
// "empties" local buffer by setting the tail of of the local logs struct to 0
uint8_t logger_pushEvent();


// Copies exp logs in bulk from local storage to mock flash
// "empties" local buffer by setting tail to 0
uint8_t logger_pushExp(struct LocalExpLogs * local_exp_logs,
                        struct ExperimentLogHeader * exp_header);

// In-memory copy of the header stored in the (mock) flash
extern struct FlashHeader FLASH_header;


// Initialize header in flash with some default values.
// Useful for setting up the flash for the first time.
void logger_initHeader();

// Copies flash header data from mock flash to flash header struct
// Can be used on startups after the first one, or perhaps for ECC tasks
// void get_flash_header(struct FlashHeader * out);
void logger_fetchHeader();

enum LogType
{
  EVENT = 0,
  EXP1 = 1,
  EXP2 = 2
};

// Updates the header in mock flash by copying struct to header
void logger_updateHeader();


// copies the oldest sector from flash as `page_buff` argument
// Returns the type of log (for ECC subsystem to inform flash driver where to write corrected page)
enum LogType logger_getOldestSector(uint8_t* sectorBuffer);

#endif
