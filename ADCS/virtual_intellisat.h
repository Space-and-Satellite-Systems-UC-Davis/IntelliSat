/**@file virtual_intellisat.h
 *
 * @brief ADCS Software's interface to Intellisat.
 *
 * This file contains the function prototype for every
 *  actuator-commanding and sensor-reading function
 *  the ADCS Software needs from Intellisat.
 *
 * Their corresponding definitions will be written in the Intellisat
 *  repo by the CS team. While this means any ADCS code that uses
 *  these functions cannot run outside of the Intellisat repo, 
 *  this approach allows for complete separation of the two repos,
 *  which vastly simplifies development for everyone.
 *
 * @author Jacob Tkeio (jacobtkeio@gmail.com)
 * @date 12/30/2023
 */

#ifndef VIRTUAL_INTELLISAT_H
#define VIRTUAL_INTELLISAT_H

typedef enum {
    GET_EPOCH_SUCCESS,
    GET_EPOCH_FAILURE
} vi_get_epoch_status;

/**@brief Report the current date and time to second accuracy.
 *
 * @param year,month,day,hour,minute,second Return-by-reference ptrs.
 *
 * @return vi_get_epoch_status A return code.
 */
vi_get_epoch_status
vi_get_epoch(
    int *year,
    int *month,
    int *day,
    int *hour,
    int *minute,
    int *second
);



typedef enum {
    GET_CURR_MILLIS_SUCCESS,
    GET_CURR_MILLIS_FAILURE
} vi_get_curr_millis_status;

/**@brief Report the value of Intellisat's millisecond counter.
 *
 * This will be used to calculate change in time for control loops.
 *
 * @param curr_millis Return-by-reference pointer.
 *
 * @return vi_get_curr_millis_status A return code.
 */
vi_get_curr_millis_status
vi_get_curr_millis(
    int *curr_millis
);



typedef enum {
    GET_ANGVEL_SUCCESS,
    GET_ANGVEL_FAILURE
} vi_get_angvel_status;

/**@brief Retrieve angular velocity data from the IMU.
 *
 * @param angvel_x,angvel_y,angvel_z Return-by-reference ptrs.
 *
 * The sign of the angular velocity values must adhere to the
 *   Right-Hand-Rule as defined by the satellite's positive axes.
 *
 * @return vi_get_angvel_status A return code.
 */
vi_get_angvel_status
vi_get_angvel(
    double *angvel_x, 
    double *angvel_y,
    double *angvel_z
);



typedef enum {
    HDD_COMMAND_SUCCESS,
    HDD_COMMAND_FAILURE
} vi_hdd_command_status;

/**@brief Send a throttle command to the HDD.
 *
 * @param throttle The desired throttle in the range [-1.0, 1.0].
 *
 * Intellisat must check these bounds for the input.
 *
 * Positive throttle must correspond to positive angular
 *   acceleration as defined by the satellite's positive Z axis
 *   and the Right-Hand-Rule.
 *
 * @return vi_hdd_command_status A return code.
 */
vi_hdd_command_status
vi_hdd_command(
    double throttle
);



typedef enum{
//Start CSS
	CSS_PX1,
	CSS_PX2,
	
	CSS_NX1,
	CSS_NX2,
	
	CSS_PY1,
	CSS_PY2,

	CSS_NY1,
	CSS_NY2,
	
	CSS_PZ1,
	CSS_PZ2,
	
	CSS_NZ1,
	CSS_NZ2,
//End CSS
	
//Start MAG
	MAG1_X,
	MAG2_X,
	
	MAG1_Y,
	MAG2_Y,
	
	MAG1_Z,
	MAG2_Z,
//End MAG
	
	
//Start IMU
	IMU1_X,
	IMU2_X,
	
	IMU1_Y,
	IMU2_Y,
	
	IMU1_Z,
	IMU2_Z,
//End IMU
} vi_sensors;

typedef enum{
	GET_CONSTANT_SUCCESS,
	GET_CONSTANT_FAILURE
} vi_get_constant_status;


/**@brief Get the current calibration values for a sensor.
 *
 * @param sensor we want calibration for.
 * @param offset and scalar Return-by-reference ptrs.
 *
 * @return vi_get_constant_status A return code, success/failure.
 */
vi_get_constant_status
vi_get_sensor_calibration(
	vi_sensors sensor, 
	float *offset,
	float *scalar
);


typedef enum{
	GET_TLE_SUCCESS_NEW,
	GET_TLE_SUCCESS_OLD,
	GET_TLE_FAILURE
} vi_get_TLE_status;

/**@brief Get the current TLE.
 *
 * @param TLE line 1 and line 2 Return-by-reference ptrs.
 *
 * @return vi_get_constant_status A return code, success/failure.
 */
vi_get_TLE_status
vi_get_TLE(
	char *tle_line1, 
	char *tle_line2
);


/**@brief Get the current status(on/off) of a given sensor.
 *
 * @param sensor we want calibration for.
 * @param status (boolean) for Return-by-reference pointer.
 *
 * @return vi_get_constant_status A return code, success/failure.
 */
vi_get_constant_status
vi_get_sensor_status(
	vi_sensors sensor,
	int *sensor_status
);

typedef enum {
	VI_GET_MAG_SUCCESS,
	VI_GET_MAG_FAILURE
} vi_get_mag_status;

vi_get_mag_status
vi_get_mag(
	double *mag_x,
	double *mag_y,
	double *mag_z
);

#endif//VIRTUAL_INTELLISAT_H
