#include "logger_to_flash.h"

#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stddef.h>

const struct FlashHeader default_flash_header = {
    .events_header = {
        .start_sector_num = EVENT_START_SECTOR,
        .end_sector_num = EVENT_START_SECTOR + EVENT_SECTORS,
        .tail = EVENT_START_SECTOR,
        .oldest_sector_num = EVENT_START_SECTOR
    },
    .exp1_header = {
        .start_sector_num = EXP1_START_SECTOR,
        .end_sector_num = EXP1_END_SECTOR,
        .tail = EXP1_START_SECTOR,
        .oldest_sector_num = EXP1_START_SECTOR,
		.extra = 0
    },
    .exp2_header = {
        .start_sector_num = EXP2_START_SECTOR,
        .end_sector_num = EXP2_END_SECTOR,
        .tail = EXP2_START_SECTOR,
        .oldest_sector_num = EXP2_START_SECTOR,
		.extra = 0
    },
    .current_exp_num = 1,
	.backup_tle_addr = 0,
	.extra = 0
};

struct FlashHeader FLASH_header = {0};


// ============================================================== //
//                 		 HEADER<-->FLASH                  		  //
// ============================================================== //

bool logger_initHeader() {
    if (!flash_eraseSector(0)) return false;
    if (!flash_writePage(0, (uint8_t*) &default_flash_header)) return false;
    memcpy(&FLASH_header, &default_flash_header, sizeof(default_flash_header));
    return true;
}

bool logger_fetchHeader() {
    if(!flash_readPage(0, (uint8_t*) &FLASH_header)) return false;
    return true;
}

// TODO 2024.10.29 Need to think about FLASH_sector_erase() requirements for FLASH_write_page()
bool logger_pushHeader() {
	struct FlashHeader empty_header = {0};
	if (memcmp(&FLASH_header, &empty_header, sizeof(struct FlashHeader)) == 0) {
		return false;
	}

	if (!flash_eraseSector(0)) return false;
    if (!flash_writePage(0, (uint8_t*) &FLASH_header)) return false;
    return true;
}


// ============================================================== //
//                 		 HEADER UPDATERS                 		  //
// ============================================================== //

static uint32_t advance_addr(
    const uint32_t start,
    const uint32_t end,
    const uint32_t addr_to_advance,
    const uint32_t increment)
{
    uint32_t offset_from_start = addr_to_advance - start;
    return start + (offset_from_start + increment) % (end - start);
}

static void logger_advanceOldestEventBlock()
{
    FLASH_header.events_header.oldest_sector_num = advance_addr(
        FLASH_header.events_header.start_sector_num,
        FLASH_header.events_header.end_sector_num,
        FLASH_header.events_header.oldest_sector_num,
        1);
}

static void logger_advanceOldestExpBlock(struct ExperimentLogHeader *exp_log_header)
{
    exp_log_header->oldest_sector_num = advance_addr(
        exp_log_header->start_sector_num,
        exp_log_header->end_sector_num,
        exp_log_header->oldest_sector_num,
        1);
}


// ============================================================== //
//                 		  LOGGER-->FLASH                		  //
// ============================================================== //

/*
If the local buffer is longer than remaining space in mock flash buffer, this will write too much.

Potential solutions:
    1. Make the size of block a multiple of local buffer size, and only transfer when local buffer is full
    2. Check if size of block being transfered > remaining space in buffer and wrap around to start.
*/
// Not implemented
uint8_t logger_pushEvent() {
    // memcpy(mock_flash_buff + flash_header.events_header.tail, local_event_logs->logs, sizeof(local_event_logs->logs));
    //FLASH_write_page(current_event_header->tail, (uint8_t*) local_event_logs->logs);
    FLASH_header.events_header.tail = advance_addr(
        FLASH_header.events_header.start_sector_num,
        FLASH_header.events_header.end_sector_num,
        FLASH_header.events_header.tail,
        1
    );

    return 0;
}

uint8_t logger_pushExp(struct LocalExpLogs* local_exp_logs,
                       struct ExperimentLogHeader* current_exp_header) {

	uint8_t log_buffer[FLASH_SECTOR_SIZE]; //there's a more efficient way to do this. look at pushEvent()
	memcpy(log_buffer, (uint8_t*)local_exp_logs->logs, sizeof(local_exp_logs->logs));

	flash_eraseSector(current_exp_header->tail);
    flash_writeSector(current_exp_header->tail, log_buffer);
    current_exp_header->tail = advance_addr(
                                    current_exp_header->start_sector_num,
                                    current_exp_header->end_sector_num,
                                    current_exp_header->tail,
                                    1
    );

    return 0;
}


// ============================================================== //
//                 		  FLASH-->LOGGER              		      //
// ============================================================== //

struct LocalExpLogs logger_fetchExps(uint16_t page) {
	struct LocalExpLogs logs_in;
	uint8_t MISO[FLASH_PAGE_SIZE];

	if (!flash_readPage(page, MISO)) {
		memset(&logs_in, 0xFF, EXP_LOG_SIZE);
	}

	uint8_t num_logs = FLASH_PAGE_SIZE / EXP_LOG_SIZE;

	for (uint8_t i = 0; i < num_logs; ++i) {
		struct ExperimentLog* current_log = &logs_in.logs[i];
		memcpy(current_log, &MISO[i * EXP_LOG_SIZE], EXP_LOG_SIZE);
	}

	return logs_in;
}


enum LogType logger_getOldestSector(uint8_t* sector_buff) {
    static enum LogType oldest_block_type = EVENT;
    uint32_t oldest_sector_num;
    switch (oldest_block_type)
    {
    case EVENT:
    {
        oldest_sector_num = FLASH_header.events_header.oldest_sector_num;
        logger_advanceOldestEventBlock();
    }
    break;

    case EXP1:
    {
        oldest_sector_num = FLASH_header.exp1_header.oldest_sector_num;
        logger_advanceOldestExpBlock(&FLASH_header.exp1_header);
    }
    break;

    case EXP2:
    {
        oldest_sector_num = FLASH_header.exp2_header.oldest_sector_num;
        logger_advanceOldestExpBlock(&FLASH_header.exp2_header);
    }
    break;
    }

    // TODO: see sector_erase problem
    // LOGGER_update_header();

    flash_readSector(oldest_sector_num, sector_buff);

    enum LogType block_type = oldest_block_type;

    oldest_block_type++;
    if (oldest_block_type > EXP2)
        oldest_block_type = EVENT;

    return block_type;
}

/**
If the local buffer is longer than remaining space in mock flash buffer, this will write too much.

Potential solutions:
    1. Make the size of block a multiple of local buffer size, and only transfer when local buffer is full
    2. Check if size of block being transfered > remaining space in buffer and wrap around to start.
*/
uint8_t logger_pushEvent()
{
    // memcpy(mock_flash_buff + flash_header.events_header.tail, local_event_logs->logs, sizeof(local_event_logs->logs));
    // FLASH_write_page(current_event_header->tail, (uint8_t*) local_event_logs->logs);
    FLASH_header.events_header.tail = advance_addr(
        FLASH_header.events_header.start_sector_num,
        FLASH_header.events_header.end_sector_num,
        FLASH_header.events_header.tail,
        1);

    return 0;
}

uint8_t logger_pushExp(struct LocalExpLogs *local_exp_logs,
                       struct ExperimentLogHeader *current_exp_header)
{

    // just a tiny lil test guys
    uint8_t temp_log[FLASH_SECTOR_SIZE];
    memcpy(temp_log, (uint8_t *)local_exp_logs->logs, sizeof(local_exp_logs->logs));

    printMsg("\n\n\rINIT PRINTOUT\n\n\r");
    for (uint32_t i = 0; i < FLASH_SECTOR_SIZE; ++i)
    {
        printMsg(" %u", temp_log[i]);
        if (i % 16 == 15)
        {
            printMsg("\n\r");
        }
    }

    printMsg("WRITING: %d", current_exp_header->tail);

    flash_eraseSector(current_exp_header->tail);
    flash_writeSector(current_exp_header->tail, temp_log);
    current_exp_header->tail = advance_addr(
        current_exp_header->start_sector_num,
        current_exp_header->end_sector_num,
        current_exp_header->tail,
        1);

    return 0;
}
