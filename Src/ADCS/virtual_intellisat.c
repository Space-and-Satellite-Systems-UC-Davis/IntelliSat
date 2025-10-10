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
#include "virtual_intellisat.h"
#include <print_scan.h>
#include <globals.h>
#include "../peripherals/IMU/ASM330LHH.h"
#include <Timers/timers.h>
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
){
	return GET_EPOCH_FAILURE;
}


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
){
	*curr_millis = getSysTime();
	return GET_CURR_MILLIS_SUCCESS;
}

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
	vi_IMU imu,
    double *angvel_x,
    double *angvel_y,
    double *angvel_z
){
	enum IMU_SELECT imu_select = (imu == VI_IMU1) ? IMU0 : IMU1;
	set_IMU(imu_select);

	*angvel_x = imu_readGyro_X();
	*angvel_y = imu_readGyro_Y();
	*angvel_z = imu_readGyro_Z();
	return GET_ANGVEL_SUCCESS;
}



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
	vi_HDD hdd,
    double throttle
){
	//TODO add hdd selection
	pwm_setDutyCycle(throttle);
	return HDD_COMMAND_SUCCESS;
}



/**@brief Get the current calibration values for a sensor.
 *
 * @param sensor we want calibration for.
 * @param offset and scalar Return-by-reference ptrs.
 * @param filter_constant attenuation constant between 0.0 - 1.0 for lowpass filter calculation;
 * 						  a greater value is a greater damp on unusually large jumps in sensor data
 *
 * @return vi_get_constant_status A return code, success/failure.
 */
vi_get_constant_status
vi_get_sensor_calibration(
	vi_sensor sensor,
	float *offset,
	float *scalar,
	float *filter_constant
){
	return GET_CONSTANT_FAILURE;
}


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
){
	return GET_TLE_FAILURE;
}


/**@brief Get the current status(on/off) of a given sensor.
 *
 * @param sensor we want calibration for.
 * @param status (boolean) for Return-by-reference pointer.
 *
 * @return vi_get_constant_status A return code, success/failure.
 */
vi_get_constant_status
vi_get_sensor_status(
	vi_sensor sensor,
	int *sensor_status
){
	return GET_CONSTANT_FAILURE;
}

/**@brief Get the current magnetic field value.
 *
 * @param mag_x,mag_y,mag_z The magnetic field vector.
 *
 * @return vi_get_mag_status A return code, success/failure.
 */
vi_get_mag_status
vi_get_mag(
	vi_MAG mag,
	double *mag_x,
	double *mag_y,
	double *mag_z
){
	//TODO add mag selection
	return VI_GET_MAG_FAILURE;
}


/**@brief Sleep for some number of milliseconds.
 *
 * @param ms The number of milliseconds to sleep for.
 *
 * @return vi_delay_ms_status A return code, success/failure.
 */
vi_delay_ms_status
vi_delay_ms(
    int ms
){
	uint64_t current = getSysTime();
	delay_ms(ms);
	if (getSysTime() >= current + (uint64_t)(ms)){
		return VI_DELAY_MS_SUCCESS;
	} else {
		return VI_DELAY_MS_FAILURE;
	}
}


/**@brief printing string
 *
 * @param string to print
 *
 */

void vi_print (const char* string){
	printMsg(string);
}


vi_control_coil_status
vi_control_coil(
	double command_x,
	double command_y,
	double command_z
){
	return VI_CONTROL_COIL_FAILURE;
}

