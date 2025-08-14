/*
 * loggers_testing.c
 *
 * October 19th, 2024
 * Anthony Surkov, Dilraj Gill
 *
 * Last updated: 11-15-2024
 * Rebased to FLASH branch to ensure functionality
 *
 * This document is testing and documentation for the REALOP 1 loggers system.
 * testFunction_loggers() is near the end.
 */

#include "experiment_logger.h"
#include "logger_to_flash.h"
#include "loggers_to_fram.h"
#include <globals.h>
#include <print_scan.h>

#include <string.h>


// Test parameters
#define LOGGERS_NUM_TESTS      5
#define FAKE_EXP_NUM_SECTORS   2  // # of sectors of fake data generated for this test
#define FAKE_EXP_TIME          (FAKE_EXP_NUM_SECTORS * 15 * 1000) // num * s * ms/s


// ========================================================================= //
//                             HELPER FUNCTIONS                              //
// ========================================================================= //

const struct FlashHeader test_default_flash_header = {
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
};

void log_fake_exp() {
	printMsg("log_fake_exp(): initializing\n");
    logger_initExp();
    logger_initHeader(); // This will not be executed here, due to test_initHeader doing it

    printMsg("log_fake_exp(): experiment starting, (%u seconds)\n", (FAKE_EXP_TIME / 1000));
    logger_startExp();
    delay_ms(FAKE_EXP_TIME);
    logger_stopExp(1);

    printMsg("log_fake_exp(): experiment ending\n");
}

void printLog(struct ExperimentLog* log) {
	printMsg("exp_num: %u\n", log->exp_num);
	printMsg("rtc_time: %u\n\n", log->rtc_time);

	printMsg("gyro_x: %u\n", log->gyro_x);
	printMsg("gyro_y: %u\n", log->gyro_y);
	printMsg("gyro_z: %u\n\n", log->gyro_z);

	printMsg("mag_x: %u\n", log->mag_x);
	printMsg("mag_y: %u\n", log->mag_y);
	printMsg("mag_z: %u\n\n", log->mag_z);

	printMsg("sunsensor_1: %u\n", log->sunsensor_1);
	printMsg("sunsensor_2: %u\n", log->sunsensor_2);
	printMsg("sunsensor_3: %u\n", log->sunsensor_3);
	printMsg("sunsensor_4: %u\n", log->sunsensor_4);
	printMsg("sunsensor_5: %u\n", log->sunsensor_5);
	printMsg("sunsensor_6: %u\n\n", log->sunsensor_6);

	return;
}

void printHeader(struct FlashHeader* header) {
	printMsg("\ncurrent_exp_num: %u\n", header->current_exp_num);
	printMsg("backup_tle_addr: %u\n", header->backup_tle_addr);

	printMsg("\nexp1 header\n");
	struct ExperimentLogHeader* exp1 = &header->exp1_header;
	printMsg("start_sector_num: %u\n", exp1->start_sector_num);
	printMsg("end_sector_num: %u\n", exp1->end_sector_num);
	printMsg("tail: %u\n", exp1->tail);
	printMsg("oldest_sector_num: %u\n", exp1->oldest_sector_num);
	printMsg("start_datetime: %u\n", exp1->start_datetime);
	printMsg("exit_status: %u\n", exp1->exit_status);
	printMsg("extra: %u\n", exp1->extra);

	printMsg("\nexp2 header\n");
	struct ExperimentLogHeader* exp2 = &header->exp2_header;
	printMsg("start_sector_num: %u\n", exp2->start_sector_num);
	printMsg("end_sector_num: %u\n", exp2->end_sector_num);
	printMsg("tail: %u\n", exp2->tail);
	printMsg("oldest_sector_num: %u\n", exp2->oldest_sector_num);
	printMsg("start_datetime: %u\n", exp2->start_datetime);
	printMsg("exit_status: %u\n", exp2->exit_status);
	printMsg("extra: %u\n", exp2->extra);

	printMsg("\nevent header\n");
	struct EventLogHeader* event = &header->events_header;
	printMsg("start_sector_num: %u\n", event->start_sector_num);
	printMsg("end_sector_num: %u\n", event->end_sector_num);
	printMsg("tail: %u\n", event->tail);
	printMsg("oldest_sector_num: %u\n\n", event->oldest_sector_num);
}

// ========================================================================= //
//                               HEADER TESTS                                //
// ========================================================================= //


// TODO Write exp overflow test
// TODO Test header functionality in between experiments
// TODO make flowchart of potential failure points


// test_initHeader, 2024.11.29
// Status: OK
// Goal: verification of header initialization. Test ran before the fake exp logged.
//
bool test_initHeader() {
	if (!logger_initHeader()) {
		printMsg("test_initHeader: header initialization failed");
		return false; //Initialize header
	}

	// Read header page (page 0)
	uint8_t page_buffer[FLASH_PAGE_SIZE];
	if (!flash_readPage(0, page_buffer)) {
		printMsg("test_initHeader: flash_readPage(0, page_buffer) failed");
		return false;
	}

	// Convert read page to header struct
	struct FlashHeader header_in;
	memcpy(&header_in, &page_buffer, sizeof(struct FlashHeader));

	// Compare to default header
	if (memcmp(&header_in, &test_default_flash_header, sizeof(struct FlashHeader)) != 0) {
		printMsg("test_initHeader: default_flash_header & header_in not the same");
		return false;
	}
	return true;
}


// test_updateHeader, 2024.11.29
// Status: OK
// Goal: verification of header update function following a logging session.
//
bool test_headerUpdate() {
	// Read header page and copy to FlashHeader struct
	uint8_t page_buffer[FLASH_PAGE_SIZE];
	if (!flash_readPage(0, page_buffer)) {
		printMsg("test_headerUpdate: flash_readPage(0, page_buffer) failed");
		return false;
	}
	struct FlashHeader header_in;
	memcpy(&header_in, &page_buffer, sizeof(struct FlashHeader));

	// Configure standard header
	struct FlashHeader header_std; // Expected standard by which to compare the header in FLASH to
	memcpy(&header_std, &test_default_flash_header, sizeof(struct FlashHeader)); // Most is standard
	header_std.exp1_header.tail = EXP1_START_SECTOR + FAKE_EXP_NUM_SECTORS; //tail update
	header_std.exp1_header.exit_status = 1; //successful exp
	header_std.current_exp_num = 2; //1st exp completed

	/*
	printMsg("\nheader_std\n");
	printHeader(&header_std);
	printMsg("\nheader_in\n");
	printHeader(&header_in);
	*/

	// Check header page against standard
	if (!memcmp(&header_std, &header_in, sizeof(struct FlashHeader)) == 0) {
		printMsg("test_headerUpdate: header_std differs from page 0");
		return false;
	}

	return true;
}


// ========================================================================= //
//                               FLASH TESTS                                 //
// ========================================================================= //

// test_loggerToFlash, 2024.11.25
// Status: OK
// Goal: verification of loggers-FLASH integration.
//
bool test_expToFlash() {
	uint8_t num_logs = FLASH_SECTOR_SIZE / EXP_LOG_SIZE; // Should evaluate to 128 (2024.11.29)
	uint8_t before_buffer[FLASH_SECTOR_SIZE]; // Used for verification of relevant FLASH sector

	struct LocalExpLogs before_logs; // This is our "answer key"
	// Manually set each log to the expected value in the first sector the fake log generates
	for (uint8_t i = 0; i < num_logs; ++i) {
		struct ExperimentLog* this_log = &before_logs.logs[i];
		uint16_t log_size = sizeof(struct ExperimentLog);

		this_log->exp_num = i; this_log->rtc_time = i;
		this_log->gyro_x = i; this_log->gyro_y = i; this_log->gyro_z = i;
		this_log->mag_x = i; this_log->mag_y = i; this_log->mag_z = i;
		this_log->sunsensor_1 = i; this_log->sunsensor_2 = i;
		this_log->sunsensor_3 = i; this_log->sunsensor_4 = i;
		this_log->sunsensor_5 = i; this_log->sunsensor_6 = i;
		this_log->extra = 0;
		this_log->extra2 = 0;

		// Feed this log structure manually into the sector-sized buffer
		memcpy(&before_buffer[i * log_size], this_log, log_size);
	}

	// Read first experiment sector
	uint8_t after_buffer[FLASH_SECTOR_SIZE];
	if (!flash_readSector(EXP1_START_SECTOR, after_buffer)) {
		printMsg("loggerToFlash(): sectorRead failed");
		return false;
	}

	// Compare the rawdata from FLASH to the manual buffer made earlier
	//printMsg("before_buffer : after_buffer");
	for (uint32_t i = 0; i < FLASH_SECTOR_SIZE; ++i) {
		//printMsg("\n%u : %u", before_buffer[i], after_buffer[i]);
		if (before_buffer[i] != after_buffer[i]) {
			printMsg("loggerToFlash(): exp log does not match expected output");
			return false;
		}
	}
	return true;
}


// test_fetchExp(), 2024.11.24
// Status: OK
// Goal: verification of loggers_fetchExp() functionality. Confirmation of loggers general
// functionality.
//
bool test_fetchExp() {
	uint16_t page = (EXP1_START_SECTOR) * FLASH_PAGES_PER_SECTOR; // Designate a page

	struct LocalExpLogs MISO_logs; // We will be fetching logs into this struct's logs
								   // LocalExpLogs->logs (8 logs per such struct, a page's worth)
	MISO_logs = loggers_fetchExps(page);

	uint8_t num_logs = FLASH_PAGE_SIZE / EXP_LOG_SIZE; // Should evaluate to 8 (2024.11.29)

	// Check all values against the expected values generated by log_fake_exp()
	for (uint8_t i = 0; i < num_logs; ++i) {
		struct ExperimentLog* this_log = &MISO_logs.logs[i];

		if (this_log->exp_num != i || this_log->rtc_time != i) {
			return false;
		}
		if (this_log->gyro_x != i || this_log->gyro_y != i || this_log->gyro_z != i) {
			return false;
		}
		if (this_log->mag_x != i || this_log->mag_y != i || this_log->mag_z != i) {
			return false;
		}
		if (this_log->sunsensor_1 != i || this_log->sunsensor_2 != i) {
			return false;
		}
		if (this_log->sunsensor_3 != i || this_log->sunsensor_4 != i) {
			return false;
		}
		if (this_log->sunsensor_5 != i || this_log->sunsensor_6 != i) {
			return false;
		}
	}
    return true;
}


// ========================================================================= //
//                                FRAM TESTS                                 //
// ========================================================================= //

void test_ADCS_constants() {
    // Make up some recognizable sun sensors data
    uint8_t sun_stuff_out[FRAM_SUNSENSORS_SIZE];
    for (uint16_t i = 0; i < FRAM_SUNSENSORS_SIZE; ++i) {
        sun_stuff_out[i] = i;
    }

    // Push it
    FRAM_writeData(FRAM_SUNSENSORS_PAGE, FRAM_SUNSENSORS_SIZE, sun_stuff_out);

    // See it
    uint8_t sun_stuff_validation[FRAM_PAGE_SIZE];
    FRAM_readPage(FRAM_SUNSENSORS_PAGE, sun_stuff_validation);
    for (uint16_t i = 0; i < FRAM_PAGE_SIZE; ++i) {
        printMsg("%u ", sun_stuff_validation[i]);
    }

    // Pull it and see it
    uint8_t sun_stuff_in[FRAM_SUNSENSORS_SIZE];
    FRAM_pullData(FRAM_SUNSENSORS_PAGE, FRAM_SUNSENSORS_SIZE, sun_stuff_in);
    for (uint16_t i = 0; i < FRAM_SUNSENSORS_SIZE; ++i) {
        printMsg("%u ", sun_stuff_in[i]);
    }

    // Check it
    for (uint16_t i = 0; i < FRAM_SUNSENSORS_SIZE; ++i) {
        if (sun_stuff_in[i] != sun_stuff_out[i]) {
            return false;
        }
    }
    return true;
}


// ========================================================================= //
//                             testFunction Cores                            //
// ========================================================================= //

void testFunction_flashloggers() {
	log_fake_exp();

    bool (*testFunctions[LOGGERS_NUM_TESTS])() = { test_fetchExp };
    const char *testNames[LOGGERS_NUM_TESTS] = { "fetchExp" };

    printMsg("\r\n\nLoggers core tests\n");

    for (uint8_t i = 0; i < LOGGERS_NUM_TESTS; ++i) {
        printMsg("\n\r%s: %s", testNames[i], testFunctions[i]() ? "OK" : "FAIL");
    }

	printMsg("\n\rtest_headerUpdate: %s", test_headerUpdate() ? "OK" : "FAIL");
}

void testFunction_framloggers() {
    test_ADCS_constants();
}
