/**
 * @file sensors.h
 *
 * @brief Interface for general sensor functions
 *
 * @author Tarini Maram (tarini.maram@gmail.com) 1/8/2025
 */
#ifndef SENSORS_H
#define SENSORS_H

#include "determination/novasc3.1/novas.h"
#include "adcs_math/vector.h"
#include "adcs_math/calibration.h"
#include "virtual_intellisat.h"

#include <limits.h>
#include <stdint.h>


typedef enum getMag{
    GET_MAG_SUCCESS,
    GET_MAG_FAILURE,
    MAG_CALIBRATION_FAILURE
} getMag_status;

/**
 * @brief Retrive sensor data from Virtual Intellisat and perform calibration 
 *
 * @param sensor the sensor to read from 
 * @param prevValue previous sensor value
 * @param currValue the current sensor value to be read 
 */
getMag_status getMag(vi_sensor sensor, vec3 prevVal, vec3 *currVal);

typedef enum getIMU{
    GET_IMU_SUCCESS,
    GET_IMU_FAILURE,
    IMU_CALIBRATION_FAILURE
} getIMU_status;

/**
 * @brief Retrive sensor data from Virtual Intellisat and perform calibration 
 *
 * @param sensor the sensor to read from 
 * @param prevValue previous sensor value
 * @param currValue the current sensor value to be read 
 */
getIMU_status getIMU(vi_sensor sensor, vec3 prevVal, vec3 *currVal);

typedef enum getCSS{
    GET_CSS_SUCCESS,
    GET_CSS_FAILURE,
    CSS_CALIBRATION_FAILURE
} getCSS_status;

/**
 * @brief Retrive sensor data from Virtual Intellisat and perform calibration 
 *
 * @param sensor the sensor to read from
 * @param face the desired sun sensor face 
 * @param prevValue previous sensor value
 * @param currValue the current sensor value to be read 
 */
getCSS_status getCSS(vi_sensor sensor ,double prevVal, double *currVal);

/**
 * returns 1 if in eclipse, 0 if not
 * threshold for eclipse is 0 - 0.25 sun sensors reading magnitude
 */
int is_in_eclipse();

/**@brief Return a choice of sensors in a pair for the current alternation.
 *
 * @param sensor the sensor pair to choose between.
 *  Either sensor in the pair (e.g. VI_CSS_PX1 and VI_CSS_PX2) can be
 *  specified here to choose the whole pair.
 * @param generation The generation number of the alternation [0-255].
 *
 * @return 1 or 2 The sensor choice. 0 If invalid sensor.
 */
int sensor_pair_choice(vi_sensor sensor, int generation);

#endif
