/*
 * loggers_interface.h
 *
 * March 7, 2026
 *   Author: Anthony Surkov
 *   Last updated: 03/07/26
 */

#ifndef LOGGERS_INTERFACE_H_
#define LOGGERS_INTERFACE_H_

// #include "flash_logger.h"
// #include "fram_logger.h"
#include "logging_records.h"

/**
 * Updates the TLE (to FRAM).
 *
 * @param data_in     TLE data to be pushed to the FRAM (140 bytes).
 *
 * @returns           Whether push was successful (bool).
 */
bool push_tle(char *data_in);

/**
 * Returns the TLE (from FRAM).
 *
 * @returns           TLE data (140 bytes).
 */
char *pull_tle();

/**
 * Updates sun sensor parameters.
 *
 * @param data_in     Sun sensor parameters (96 bytes).
 *
 * @returns           Whether push was successful (bool).
 */
bool push_ss(uplink_record_calibration_css data_in);

/**
 * Returns sun sensor parameters.
 *
 * @returns           Sun sensor parameters (96 bytes).
 */
uplink_record_calibration_css pull_ss();

/**
 * Updates MAG parameters.
 *
 * @param data_in     MAG parameters (48 bytes).
 *
 * @returns           Whether push was successful (bool).
 */
bool push_mag(uplink_record_calibration_mag data_in);

/**
 * Returns MAG parameters.
 *
 * @returns           MAG parameters (48 bytes).
 */
uplink_record_calibration_mag pull_mag();

/**
 * Updates IMU parameters.
 *
 * @param data_in     IMU parameters (48 bytes).
 *
 * @returns           Whether push was successful (bool).
 */
bool push_imu(uplink_record_calibration_imu data_in);

/**
 * Returns IMU parameters.
 *
 * @returns           IMU parameters (48 bytes).
 */
uplink_record_calibration_imu pull_imu();

bool push_pid(uplink_record_calibration_pid data_in);

uplink_record_calibration_pid pull_pid();

/**
 * Pushes one data buffer to the FLASH. Page incrementing/storage location
 * is taken care of automatically.
 *
 * @param data_out     Buffer of data to push to FLASH (256 or 4096 bytes).
 * @param data_size    Must be 256 or 4096 bytes; else function will
 * return 1.
 *
 * @returns            Whether push was successfully completed (bool).
 */
bool flash_push(uint8_t *data_out, uint16_t data_size);

/**
 * Pushes one data buffer to the FRAM. Page increments are not automatic.
 * Note that, for most intents and purposes, this function is abstracted as
 * push_tle, push_ss, push_mag, and push_imu.
 *
 * @returns            Whether push was successfully completed (bool).
 */
bool fram_push(uint8_t *data_out, uint16_t data_size, uint16_t page);

/**
 * Pulls one data buffer from the FLASH, from the least-recently-visited
 * location. Page incrementings/storage location is taken care of automatically.
 *
 * @param data_in      Buffer for data collection (4096 bytes).
 */
uint32_t *flash_pull(uint8_t *data_in);

/**
 * Pulls one data buffer from the FRAM.
 * Note that, for most intents and purposes, this function is abstracted as
 * pull_tle, pull_ss, pull_mag, and pull_imu.
 */
uint32_t *fram_pull(uint8_t *data_in, uint16_t page);

/**
 * Push sensor status
 *
 * @returns Whether the push was successful
 */
bool push_sensor_status(uplink_record_sensor_enable data_in);

/**
 * Returns sensor status
 *
 * @returns Returns sensor status data (4 bytes)
 */
uplink_record_sensor_enable pull_sensor_status();

/**
 * Returns the experiment generation count
 *
 *  @returns            Returns the current experiment generation
 */
uint8_t pull_experiment_generation();

/**
 * Updates the experiment generation count
 *
 *  @returns           Returns whether the push was successful
 */
bool push_experiment_generation(uint8_t data_in);

/**
 * Returns the detumbling generation count
 *
 * @returns            Returns the current detumbling generation
 */
uint8_t pull_detumbling_generation();

/**
 * Updates the detumbling generation count
 *
 * @returns           Returns whether the push was successful
 */
bool push_detumbling_generation(uint8_t data_in);

/**
 * Returns the determination generation count
 *
 * @returns Returns the current determination generation
 */
uint8_t pull_determination_generation();

/**
 * Updates the determination generation count
 *
 * @returns Returns whether the push was successful
 */
bool push_determination_generation(uint8_t data_in);

#endif