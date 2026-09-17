/**
 * @file virtual_loggers.h
 *
 * @brief ADCS Software's interface to Loggers
 *
 *  This file contain function headers for every function
 *  ADCS needs to interface with
 *
 * @author Li, Chun Ho (lchli@ucdavis.edu)
 * @date 05/14/2026
 */

#ifndef VIRTUAL_LOGGERS_H
#define VIRTUAL_LOGGERS_H

#include "../data_management/lfs_config.h"
#include "adcs_math/vector.h"
#include <stdint.h>
#include <stdio.h>

// Every row of a log file is padded to the same width so that readRecord() can
// seek straight to a row instead of scanning for newlines.
#define LOG_TS_WIDTH 19  // YYYY-MM-DD_HH-MM-SS
#define LOG_NUM_WIDTH 13 // "%13.5e", wide enough for any finite double
#define LOG_ROW_MAX 155  // the largest row_size of any file_tag

typedef struct {
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
} TIMESTAMP;

/**
 * @brief Tag denoting which kind of log file
 *
 * @param detumblingLOG
 * @param detumblingSTART TIME, IMU, MAG, TEMP
 *
 */
typedef enum {
    detumbleLOG = 0,   // IMU, MAG,
    detumbleSTART = 1, // TIME, IMU, MAG, TEMP
} file_tag;

typedef struct {
    vec3 mag;
    vec3 imu;
    vec3 mdm;
} detumbleLOGdata;

typedef struct {
    vec3 imu;
    vec3 mag;
    double temp;
} detumbleSTARTdata;

/**
 * @brief Create and open the file for logging
 *
 * @param file littlefs file handle
 * @param row index of the next row readRecord() will return
 * @param row_size the size of each row (uniform across the file)
 * @param header A tag denoting which kind of log file
 */
typedef struct {
    lfs_file_t file;
    uint32_t row;
    size_t row_size;
    file_tag header;
} LOG_FILE;

/**
 * @brief open the file for logging
 *
 * @param name name of the file
 * @param tag A tag denoting which kind of log file this is
 *
 * @note Only one log file may be open at a time. Returns NULL if one already
 *       is, or if the filesystem rejects the open.
 *
 * @return a LOG_FILE pointer
 */
LOG_FILE *createFile(char *name, file_tag tag);

/**
 * @brief open the file for logging
 *
 * @param name name of the file
 *
 * @note Only one log file may be open at a time. Returns NULL if one already
 *       is, if the file is missing, or if its header row is unrecognized.
 *
 * @return a LOG_FILE pointer
 */
LOG_FILE *openFile(char *name);

/**
 * @brief Close the log file
 *
 * @param file LOG_FILE pointer
 * @param condition The closing condition of the file
 *
 * This function should write out the final row of the log file before closing
 *
 * YYYY-MM-DD-HH-MM-SS, EXIT_LOG_FILE, <Completion / Error Message>
 */
void closeFile(LOG_FILE *file, char *message);

/**
 * Example File Format:
 * Filename: "YYYY-MM-DD_HH-MM-SS_detumble.csv"
 *
 * File Content:
 * detumblingLOG, IMU01, IMU02, MAG01, MAG02\n
 * YYYY-MM-DD_HH-MM-SS, xxx, xxx, xxx \n
 * YYYY-MM-DD_HH-MM-SS, xxx, xxx, xxx \n
 * YYYY-MM-DD_HH-MM-SS, xxx, xxx, xxx \n
 * YYYY-MM-DD-HH-MM-SS, EXIT_LOG_FILE, Completed
 *
 * Essentially we're trying to make csv file to log the data
 */

/**
 * @brief Write the header section of the log file
 *
 * This is to create a header so that the data becomes more readable
 *
 * The first entry would be the
 *
 * @param file LOG_FILE pointer
 *
 * Example: detumblingLOG, IMU01, IMU02, MAG01, MAG02\n
 */
void logHeader(LOG_FILE *file);

/**
 * @brief Append a new entry to the log file
 *
 * @param file LOG_FILE pointer
 * @param record the data to store in the
 *
 * This function should attach a timeStamp at the beginning of every log entry.
 *
 * Example: YYYY-MM-DD_HH-MM-SS, <record>
 */
void logRecord(LOG_FILE *file, void *record);

/**
 * @brief Return all the bytes of a log file
 *
 * @param file LOG_FILE pointer
 *
 * @note The returned pointer refers to a shared static buffer that the next
 *       readHeader() or readRecord() call overwrites. It is not terminated.
 *
 * @return Byte buffer storing contents of the file
 */
uint8_t *readHeader(LOG_FILE *file);

/**
 * @brief Return all the bytes of a log file
 *
 * @param file LOG_FILE pointer
 *
 * @note The returned pointer refers to a shared static buffer that the next
 *       readHeader() or readRecord() call overwrites. It is not terminated.
 *       Returns NULL once every row has been read.
 *
 * @return Byte buffer storing contents of the file
 */
uint8_t *readRecord(LOG_FILE *file);

/**
 * @brief List the log files currently in the file system
 *
 * @note The returned pointer refers to a shared static buffer that the next
 *       lsRecord() call overwrites. The listing is truncated if it does not
 *       fit.
 *
 * @return string of the list of log files
 */
char *lsRecord();

#endif // VIRTUAL_LOGGERS_H