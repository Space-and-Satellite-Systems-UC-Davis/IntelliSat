/*
 * logger_to_flash.h (Logger-to-FLASH Interface)
 *
 * Authors: Dilraj, Anthony
 * Last updated: 2024.11.21
 * Description: FLASH utilities belonging to the loggers system
 */

#ifndef MOCKUP_FLASH_FLASH_H_
#define MOCKUP_FLASH_FLASH_H_


#include "event_logger.h"
#include "experiment_logger.h"

#include <stdint.h>
#include "../peripherals/FLASH/W25Q128JV.h"
#include "../tools/print_scan.h"


#define HEADER_SECTOR        0

#define EVENT_SECTORS        5
#define EXP_SECTORS          5

#define EVENT_START_SECTOR   1

#define EXP1_START_SECTOR    EVENT_START_SECTOR + EVENT_SECTORS
#define EXP1_END_SECTOR      EXP1_START_SECTOR + EXP_SECTORS

#define EXP2_START_SECTOR    EXP1_START_SECTOR + EVENT_SECTORS
#define EXP2_END_SECTOR      EXP2_START_SECTOR + EXP_SECTORS


#pragma pack(push,1)
// Event Header: used for event metadata
struct EventLogHeader
{
    uint32_t start_sector_num: 24;
    uint32_t end_sector_num: 24;
    uint32_t tail: 24;
    uint32_t oldest_sector_num: 24;
};
// Experiment Header: used for exp metadata
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
// FLASH Header: used for log metadata
struct FlashHeader
{
    struct EventLogHeader events_header;
    struct ExperimentLogHeader exp1_header;
    struct ExperimentLogHeader exp2_header;

    uint8_t current_exp_num: 8;
    uint32_t backup_tle_addr: 24;
    uint8_t extra[198];
};
#pragma pack(pop)

// In-memory copy of the FLASH header
extern struct FlashHeader FLASH_header;


// ============================================================== //
//                 		 HEADER<-->FLASH                  		  //
// ============================================================== //

/**
 * Initializes FLASH header.
 * Copies default values from default_flash_header into FLASH_header and writes it to page 0.
 */
bool logger_initHeader();


/**
 * Fetches flash header from the FLASH into the FLASH_header struct.
 * Reads most recent header, as tracked in FLASH_header.current_header_num.
 */
bool logger_fetchHeader();


/**
 * Pushes header to FLASH.
 * Calls logger_overflowHeader() if FLASH header sector is full. Writes to page 0 afterwards.
 */
bool logger_pushHeader();


// ============================================================== //
//                 		  LOGGER-->FLASH                		  //
// ============================================================== //

/**
 * Event Logger (TODO)
 * Copies event logs in bulk from local storage to FLASH.
 * "Empties" local buffer by setting the tail of the local logs struct to 0.
 */
uint8_t logger_pushEvent();


/**
 * Experiment Logger
 * Copies experiment logs in bulk from local storage to FLASH.
 * "Empties" local buffer by setting the tail of the local logs struct to 0.
*/
uint8_t logger_pushExp(struct LocalExpLogs * local_exp_logs,
                       struct ExperimentLogHeader * exp_header);


// ============================================================== //
//                 		  FLASH-->LOGGER              		      //
// ============================================================== //

/**
 * Logger fetcher
 * Returns the 8 constituent experiment logs of an argued page.
 */
struct LocalExpLogs loggers_fetchExps(uint16_t page);

/**
 * Fetches oldest sector from FLASH as sectorBuffer argument.
 * Returns the type of log (for ECC subsystem to inform FLASH where to write corrected page)
 */
enum LogType
{
  EVENT = 0,
  EXP1 = 1,
  EXP2 = 2
};
enum LogType logger_fetchOldestSector(uint8_t* sectorBuffer);

#endif
