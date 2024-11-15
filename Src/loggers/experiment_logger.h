#ifndef LOGGERS_EXPERIMENT_LOGGER_H_
#define LOGGERS_EXPERIMENT_LOGGER_H_

#include "../peripherals/FLASH/W25Q128JV.h"

#ifdef NO_OP
#include <stdbool.h>
#else
#include <globals.h>
#endif

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
//#include <stdbool.h>


// 128 bits == 16 bytes
#pragma pack(push,1)
// struct ExperimentLog
// {
//     uint16_t rtc_time: 12;
//     int16_t gyro_x: 16, gyro_y: 16, gyro_z: 16;
//     int16_t dgyro_x: 16, dgyro_y: 16, dgyro_z: 16;
//     uint32_t  extra: 20;
// };

//m ExperimentLog
struct ExperimentLog
{
    uint16_t exp_num;
    uint16_t rtc_time: 12;

    int16_t gyro_x: 16,
        gyro_y: 16,
        gyro_z: 16;

    int16_t mag_x: 16,
        mag_y: 16,
        mag_z: 16;

    int16_t sunsensor_1: 12,
        sunsensor_2: 12,
        sunsensor_3: 12,
        sunsensor_4: 12,
        sunsensor_5: 12,
        sunsensor_6: 12;

    uint8_t extra: 4;
    uint64_t extra2: 52;
};

#pragma pack(pop)

#define LOCAL_EXP_LOG_COUNT (FLASH_SECTOR_SIZE/sizeof(struct ExperimentLog))

struct LocalExpLogs {
    bool paused;
    unsigned int num_logs;
    unsigned int tail;
    struct ExperimentLog logs[LOCAL_EXP_LOG_COUNT];
};

void logger_initExp();

void logger_startExp();

void logger_stopExp(int experiment_status);


// Currently for synthetic data purposes
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
);

#endif
