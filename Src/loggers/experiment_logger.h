#ifndef LOGGERS_EXPERIMENT_LOGGER_H_
#define LOGGERS_EXPERIMENT_LOGGER_H_

#include <stdint.h>
#include <stdio.h>

#define LOCAL_EXP_LOG_BUFFER_SIZE (2 * 32)

union ExperimentLog
{
    // 128 bits == 16 bytes
    #pragma pack(push,1)
    struct
    {
        uint16_t rtc_time: 12;
        int16_t gyro_x: 16, gyro_y: 16, gyro_z: 16;
        int16_t dgyro_x: 16, dgyro_y: 16, dgyro_z: 16;
        uint32_t  extra: 20;
    } as_struct;
    #pragma pack(pop)
    uint64_t as_arr[2];
};

struct UnpackedExpLog
{
    uint16_t rtc_time;
    int16_t gyro_x, gyro_y, gyro_z;
    int16_t dgyro_x, dgyro_y, dgyro_z;
    uint32_t  extra;
};

struct LocalExpLogs {
    unsigned int buffer_size;
    unsigned int tail;
    uint64_t buffer[LOCAL_EXP_LOG_BUFFER_SIZE];
};

int is_exp_being_logged();

void start_exp_logging();

void stop_exp_logging(int experiment_status);

void handle_exp_overflow();

// Takes a uint64_t[2] and appends to the local buffer
void add_exp_log( uint64_t exp_log[2], struct LocalExpLogs * local_exp_logs );

uint8_t build_and_add_exp_log(
    unsigned int rtc_time,     // Date+Hr+Min+Sec
    int16_t  gyro_x,
    int16_t  gyro_y,
    int16_t  gyro_z,
    int16_t  dgyro_x,
    int16_t  dgyro_y,
    int16_t  dgyro_z,
    unsigned int parity,
    struct LocalExpLogs * local_exp_logs
);

uint8_t get_exp_log(uint64_t addr, uint64_t const exp_log_buff[], union ExperimentLog * const retrieved_log);

int detect_exp_buff_overflow();

void handle_exp_buff_overflow();



#endif