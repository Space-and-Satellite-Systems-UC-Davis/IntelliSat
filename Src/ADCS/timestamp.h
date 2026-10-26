/**
 * @file timestamp.h
 *
 * @brief Broken-down wall-clock time, shared by the sensor and logger layers.
 *
 * This lives on its own so that adcs_math/sensors.h can format timestamps
 * without pulling in virtual_loggers.h, which drags the whole littlefs/QSPI
 * stack (and with it the STM32 headers) into every host-side build.
 */

#ifndef TIMESTAMP_H
#define TIMESTAMP_H

typedef struct {
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
} TIMESTAMP;

#endif // TIMESTAMP_H
