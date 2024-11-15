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
#include <globals.h>
#include <print_scan.h>


// Defines for test parameters.
#define LOGGERS_NUM_TESTS    1

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-//
/*                             HELPER FUNCTIONS                              */
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-//



//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-//
/*                            FUNCTIONALITY TESTS                            */
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-//

bool temptest_readJedecID() {
	uint8_t id_receiver[3];
	qspi_setCommand(
	  QSPI_FMODE_INDIRECT_READ, //read
	  QSPI_1_WIRE, //1 wire for instruction
	  QSPI_UNUSED, //no wire for address
	  QSPI_UNUSED, //no wire for alt bytes
	  QSPI_UNUSED, //0 dummy cycles for JEDEC
	  QSPI_1_WIRE, //1 wire for data
	  false //no DMA
	);
	qspi_sendCommand(
	  0x9F, //JEDEC ID
	  QSPI_UNUSED, //no address
	  3, //3 bytes data
	  id_receiver, //where data goes
	  QSPI_READ,
	  QSPI_TIMEOUT_PERIOD
	);

	if (id_receiver[0] != 0xEF) {
		printMsg("readJedecID: unexpected ID, expected 0xEF4018\n\r");
		return false;
	}
	if (id_receiver[1] != 0x40) {
		printMsg("readJedecID: unexpected ID, expected 0xEF4018\n\r");
		return false;
	}
	if (id_receiver[2] != 0x18) {
		printMsg("readJedecID: unexpected ID, expected 0xEF4018\n\r");
		return false;
	}

	flash_eraseSector(1);

	return true;
}


// IP test_expToFlash(). Oct 19 2024
// Goal: Integration of experiment logging with FLASH interface.
//
bool test_expToFlash() {

    logger_initExp();
    logger_initHeader(); //integrate into exp_init()

    logger_startExp();
    delay_ms(30000);
    logger_stopExp(1);

	uint8_t sectorBuffer[FLASH_SECTOR_SIZE];
	uint32_t totalLogSectors = EVENT_SECTORS + (2 * EXP_SECTORS);

	printMsg("\n\nTOTAL LOG SECTORS: %u", totalLogSectors);
	uint32_t totalLogsFound = 0;

	for (uint32_t i = 0; i < totalLogSectors; ++i) {
		enum LogType testingBlockType;
		testingBlockType = logger_getOldestSector(sectorBuffer);

		if (testingBlockType == EXP1) {
			++totalLogsFound;

			/*printMsg("\n\n\rRAWDATA\n\n\r");
			for (uint32_t j = 0; j < FLASH_SECTOR_SIZE; ++j) {
				printMsg("%u ", sectorBuffer[j]);
				if (j % 16 == 0) {
					printMsg("\n\r");
				}
			}*/

			printMsg("\n\n\rRAW SECTOR AFTER READ\n\n\r");
			    for (uint32_t i = 0; i < FLASH_SECTOR_SIZE; ++i) {
			    	printMsg(" %u", sectorBuffer[i]);
			    	if (i % 16 == 0 && i != 0) { printMsg("\n\r"); }
			    }

			printMsg("\n\n\rDATA\n\n\r");
			for (uint32_t j = 0; j < FLASH_SECTOR_SIZE; j += 32) {
				printMsg("Log %u: ", j / 32);

				uint8_t temp_buffer[32];
				memcpy(temp_buffer, &sectorBuffer[j], 32);

				uint16_t raw_value = *(uint16_t*)&temp_buffer[0]; //jank af
				printMsg("\n\r\tRaw: %hu", raw_value);
				printMsg("\n\r\tHex: %04X", raw_value);

				struct ExperimentLog* retrieved_log = (struct ExperimentLog*)temp_buffer;
				printMsg("\n\r\tStruct: %hu", retrieved_log->exp_num);

				printMsg("\n");
			}
		}
	}

	printMsg("\n\nTOTAL LOGS FOUND: %u", totalLogsFound);

    return true;
}


//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-//
/*                             testFunction Core                             */
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-//

void testFunction_loggers() {
/*
    bool (*testFunctions[LOGGERS_NUM_TESTS])() = { test_expToFlash };

    const char *testNames[LOGGERS_NUM_TESTS] = { "expToFlash" };


    printMsg("\r\n\nLoggers core tests");

    for (uint8_t i = 0; i < LOGGERS_NUM_TESTS; ++i) {
        printMsg("\n\r%s: %s", testNames[i], testFunctions[i]() ? "OK" : "FAIL");
    }
    */
	temptest_readJedecID();

}
