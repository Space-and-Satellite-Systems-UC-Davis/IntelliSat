/**
 * @file calibration.h
 *
 * @brief Utility functions for calibrating sensor input
 *
 * @author Chun Ho Li (lchli@ucdavis.edu) 6/23/2025
 */

#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <stdint.h>

/**
 * @brief Implement lowpass filter on sensor raw values to mitigate the effect
 * of noise from abnormally high values
 *
 * @param currValue current sensor raw value
 * @param prevValue previous sensor raw value
 * @param filterConstant constant between 0-1; a greater value is a greater damp
 * on unusually large jumps in sensor data
 *
 * @return filtered sensor value
 */
float lowpass_filter(float currValue, float prevValue, float filterConstant);

/**
 * @brief Calculates sensor calibration value after filtering data through
 * simple lowpass filter with default constant 0.5
 *
 * @param currValue current sensor raw value
 * @param prevValue previous sensor raw value
 * @param offset constant the sensor reads when it is not supposed to read
 * anything
 * @param scalar constant multiplier for sensor raw values
 * @param filterConstant attenuation constant between 0-1 for lowpass filter; a
 * greater value is a greater damp on unusually large jumps in sensor data
 *
 * @return sensor calibration value after filtration
 */
float get_sensor_calibration(float currValue, float prevValue,
                             float offset, float scalar, float filterConstant);


/**
 * @brief Safely calculate delta_t accounting for integer overflow
 *
 * @param currTime current timestamp in milliseconds
 * @param prevTime previous timestamp in milliseconds
 *
 * @return delta_t change in time
 */
uint64_t get_delta_t(uint64_t currTime, uint64_t prevTime);

#endif