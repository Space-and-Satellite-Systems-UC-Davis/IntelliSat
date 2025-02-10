#include "downlink_logs.h"
#include "logger_to_flash.h"
#include "experiment_logger.h"

// #include "../tools/print_scan.h"
//#define printMsg(x) printf(x)

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

static struct LocalExpLogs logger_local_exp_logs = {
    .paused = true,
    .num_logs = LOCAL_EXP_LOG_COUNT,
    .tail = 0,
    .logs = {{0, 0, 0, 0, 0, 0, 0, 0}}
};

static struct ExperimentLogHeader* logger_current_exp_header;

#ifdef NO_OP
//#include <stdbool.h>
#else
#include <Timers/timers.h>
#include <globals.h>
#endif

static void logger_logCurrentExp();

void logger_initExp() {
	logger_initExpTimer();
	logger_registerLogFunction(logger_logCurrentExp);
}

// TODO: figure out what to set up when starting logging
void logger_startExp() {

    int current_exp = FLASH_header.current_exp_num;

    if(current_exp > EXP2) {
        current_exp = EXP1;
    }

    switch(current_exp) {
        case EXP1: {
            logger_current_exp_header = &FLASH_header.exp1_header;
            break;
        }
        case EXP2: {
            logger_current_exp_header = &FLASH_header.exp2_header;
            break;
        }
    }

    logger_local_exp_logs.paused = false;
    logger_expTimerOn();
}

// TODO: Save experiment status to flash header
void logger_stopExp(int experiment_status) {
    logger_expTimerOff();

    logger_current_exp_header->exit_status = experiment_status;
    int current_exp = FLASH_header.current_exp_num + 1;

    if(current_exp > EXP2) {
        current_exp = 1;
    }

    FLASH_header.current_exp_num = current_exp;

    // TODO: handle pushHeader failing
    logger_pushHeader();
}

// TODO: swap out the local buffer used for local logging on overflow
static void logger_handleExpOverflow() {
    logger_local_exp_logs.paused = true;

    //logger_downlinkExpLogs(&logger_local_exp_logs);

    // TODO: Should unpausing go into an interrupt handler for comms (0.5) or flash (1.0)?
    logger_local_exp_logs.paused = false;
    logger_pushExp(&logger_local_exp_logs, logger_current_exp_header);

}

/**
 * Builds an experiment log from its arguments and passes them to add_exp_log()
 * Checks that the fields fit in the bitfields
 * Might just be useful for testing
 */
uint8_t logger_logExp(
    uint16_t exp_num,
    uint16_t rtc_time,     // Date+Hr+Min+Sec

    int16_t  gyro_x,
    int16_t  gyro_y,
    int16_t  gyro_z,

    int16_t mag_x,
    int16_t mag_y,
    int16_t mag_z,

    int16_t sunsensor_1,
    int16_t sunsensor_2,
    int16_t sunsensor_3,
    int16_t sunsensor_4,
    int16_t sunsensor_5,
    int16_t sunsensor_6,

    uint8_t extra
) {
    if (logger_local_exp_logs.paused) {
        return 1;
    }


    uint64_t current_log_index = logger_local_exp_logs.tail;


    logger_local_exp_logs.logs[current_log_index].exp_num = exp_num;
	logger_local_exp_logs.logs[current_log_index].rtc_time = rtc_time;

	logger_local_exp_logs.logs[current_log_index].gyro_x = gyro_x;
    logger_local_exp_logs.logs[current_log_index].gyro_y = gyro_y;
    logger_local_exp_logs.logs[current_log_index].gyro_z = gyro_z;

    logger_local_exp_logs.logs[current_log_index].mag_x = mag_x;
    logger_local_exp_logs.logs[current_log_index].mag_y = mag_y;
    logger_local_exp_logs.logs[current_log_index].mag_z = mag_z;

    logger_local_exp_logs.logs[current_log_index].sunsensor_1 = sunsensor_1;
    logger_local_exp_logs.logs[current_log_index].sunsensor_2 = sunsensor_2;
    logger_local_exp_logs.logs[current_log_index].sunsensor_3 = sunsensor_3;
    logger_local_exp_logs.logs[current_log_index].sunsensor_4 = sunsensor_4;
    logger_local_exp_logs.logs[current_log_index].sunsensor_5 = sunsensor_5;
    logger_local_exp_logs.logs[current_log_index].sunsensor_6 = sunsensor_6;

    logger_local_exp_logs.logs[current_log_index].extra = 0;
    logger_local_exp_logs.logs[current_log_index].extra2 = 0;

    current_log_index++;

    if ( current_log_index >= logger_local_exp_logs.num_logs ) {
        logger_local_exp_logs.tail = 0;

        logger_handleExpOverflow();
    } else {
        logger_local_exp_logs.tail = current_log_index;
    }
    return 0;
}

static void logger_logCurrentExp() {
	static uint8_t counter = 0;
	logger_logExp(counter, counter, counter, counter,
				   counter, counter, counter, counter,
				   counter, counter, counter, counter,
				   counter, counter, counter);
	++counter;
}

// Function for adding an experiment log using bitwise operations. This may be the way we have to move forward
/*
uint8_t build_exp_2(
    uint16_t rtc_time,     // Date+Hr+Min+Sec
    int16_t  gyro_x,
    int16_t  gyro_y,
    int16_t  gyro_z,
    int16_t  dgyro_x,
    int16_t  dgyro_y,
    int16_t  dgyro_z,
    uint32_t extra,
    struct LocalExpLogs * local_exp_logs
) {
    if (   !fits_in_bits(rtc_time, 12)
        || !fits_in_bits(extra, 20)
    ) {
        // Gave too large a value somewhere :(
        return 1;
    }

    uint64_t log[2] = {0, 0};

    log[0] |= ((uint64_t)rtc_time << 52);
    log[0] |= ((uint64_t)gyro_x << 36);
    log[0] |= ((uint64_t)gyro_y << 20);
    log[0] |= ((uint64_t)gyro_z << 4);
    log[0] |= ((uint64_t)dgyro_x >> 12);

    log[1] |= ((uint64_t)dgyro_y << 52);
    log[1] |= ((uint64_t)dgyro_z << 36);
    log[1] |= extra;


    add_exp_log(log, local_exp_logs);
    return 0;
}
*/

/*
// Turns a pair of uint64_t into a single experiment log with bitwise operations.
// Currently unused - using bitfields, unions, and memcpy.
struct UnpackedExpLog decode(uint64_t log_0, uint64_t log_1) {
    struct UnpackedExpLog unpacked = {
        .rtc_time = (log_0 >> 52) & MASK_12_BIT,
        .gyro_x   = (log_0 >> 36) & MASK_16_BIT,
        .gyro_y   = (log_0 >> 20) & MASK_16_BIT,
        .gyro_z   = (log_0 >> 4)  & MASK_16_BIT,
        .dgyro_x  = (log_0 & MASK_4_BIT) + ((log_1 >> 52) & MASK_12_BIT),
        .dgyro_y  = (log_1 >> 36) & MASK_16_BIT,
        .dgyro_z  = (log_1 >> 20) & MASK_16_BIT,
        .extra    = (log_1 & MASK_20_BIT)
    };

    return unpacked;
}
*/
/*
uint8_t logger_getExpLog(uint64_t addr, struct ExperimentLog * retrieved_log) {
    if (addr > logger_local_exp_logs.num_logs) {
        // printf("idx %lu fail\n", idx);
        return -1;
    } else {
        *retrieved_log = logger_local_exp_logs.logs[addr];
        return 0;
    }
}
*/
