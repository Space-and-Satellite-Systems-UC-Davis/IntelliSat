/**
 * @file virtual_intellisat.h
 *
 * @brief ADCS Software's interface to Intellisat.
 *
 *  This file contains the function prototype for every
 *  actuator-commanding and sensor-reading function
 *  the ADCS Software needs from Intellisat.
 *
 *  Their corresponding definitions will be written in the Intellisat
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

#include "ADCS.h"

#include <stdint.h>


/*################ SENSORS AND ACTUATORS ################*/

typedef enum {
    VI_MAG1 = 1,
    VI_MAG2 = 2
} vi_MAG_choice;

typedef enum {
    VI_IMU1 = 1,
    VI_IMU2 = 2
} vi_IMU_choice;

typedef enum {
	VI_CSS1 = 1,
	VI_CSS2 = 2,
	/* Kept here to avoid conflict with sensor_pair_choice */
	// VI_CSS_PX = 3,
	// VI_CSS_NX = 4,
	// VI_CSS_PY = 5,
	// VI_CSS_NY = 6,
	// VI_CSS_PZ = 7,
	// VI_CSS_NZ = 8
} vi_CSS_choice;

typedef enum {
	VI_CSS_PX = 1,
	VI_CSS_NX = 2,
	VI_CSS_PY = 3,
	VI_CSS_NY = 4,
	VI_CSS_PZ = 5,
	VI_CSS_NZ = 6
} vi_CSS_face;

typedef enum {
    VI_HDD1 = 1,
    VI_HDD2 = 2
} vi_HDD_choice;

typedef enum {
	VI_TEMP_PX = 1,
	VI_TEMP_NX = 2,
	VI_TEMP_PY = 3,
	VI_TEMP_NY = 4,
    VI_TEMP_NZ = 5
} vi_tmp_choice;

typedef enum {
	VI_SP_PX = 1,
	VI_SP_NX = 2,
	VI_SP_PY = 3,
	VI_SP_NY = 4
} vi_sol_choice;

typedef enum {
	VI_MAG_X1 = 1,
	VI_MAG_Y1 = 2,
	VI_MAG_Z1 = 3,

	VI_MAG_X2 = 4,
	VI_MAG_Y2 = 5,
	VI_MAG_Z2 = 6
} vi_MAG_value;

typedef enum {
	VI_IMU1_X = 1,
	VI_IMU1_Y = 2,
	VI_IMU1_Z = 3,
	
	VI_IMU2_X = 4,
	VI_IMU2_Y = 5,
	VI_IMU2_Z = 6
} vi_IMU_value;

typedef enum {
	VI_CSS_PX1 = 1,
	VI_CSS_NX1 = 2,
	VI_CSS_PY1 = 3,
	VI_CSS_NY1 = 4,
	VI_CSS_PZ1 = 5,
	VI_CSS_NZ1 = 6,

	VI_CSS_PX2 = 7,
	VI_CSS_NX2 = 8, 
	VI_CSS_PY2 = 9,
	VI_CSS_NY2 = 10,
	VI_CSS_PZ2 = 11,
	VI_CSS_NZ2 = 12
} vi_CSS_value;

typedef enum {
    VI_COMP_MAG_CHOICE = 1,
    VI_COMP_MAG_VALUE = 2,
    VI_COMP_IMU_CHOICE = 3,
    VI_COMP_IMU_VALUE = 4,
    VI_COMP_HDD_CHOICE = 5,
    VI_COMP_CSS_CHOICE = 6,
    VI_COMP_CSS_VALUE = 7,
    VI_COMP_TMP_CHOICE = 8,
    VI_COMP_SOL_CHOICE = 9
} vi_component;

typedef union {
    vi_MAG_choice mag_choice;
    vi_MAG_value  mag_value;
    vi_IMU_choice imu_choice;
    vi_IMU_value  imu_value;
    vi_HDD_choice hdd_choice;
    vi_CSS_choice css_choice;
    vi_CSS_value  css_value;
    vi_tmp_choice temp_sensor;
    vi_sol_choice solar_panel;
} vi_field;

typedef struct {
    vi_component component;
    vi_field field;
} vi_sensor;


/*################## ACTUATOR COMMANDS ##################*/

typedef enum {
    HDD_COMMAND_SUCCESS,
    HDD_COMMAND_FAILURE
} vi_hdd_command_status;

/**
 * @brief Send a throttle command to the HDD.
 *
 * @param hdd Which HDD to command.
 * @param throttle The desired throttle in the range [-1.0, 1.0].
 *
 * Intellisat must check these bounds for the input.
 *
 * Positive throttle must correspond to positive angular
 *  acceleration as defined by the satellite's positive Z axis
 *  and the Right-Hand-Rule.
 *
 * @return vi_hdd_command_status A return code.
 */
vi_hdd_command_status
vi_hdd_command(
    vi_HDD_choice hdd,
    double throttle
);


typedef enum {
	VI_CONTROL_COIL_SUCCESS,
	VI_CONTROL_COIL_FAILURE
} vi_control_coil_status;

/**
 * @brief Set the coils' dipole vector.
 *
 * @param command_x,command_y,command_z The dipole vector.
 *
 * @return vi_control_coil_status A return code, success/failure.
 */
vi_control_coil_status 
vi_control_coil(
	double command_x,
	double command_y,
	double command_z
);


/*################### SENSOR READINGS ###################*/

typedef enum {
    GET_EPOCH_SUCCESS,
    GET_EPOCH_FAILURE
} vi_get_epoch_status;

/**
 * @brief Report the current date and time to second accuracy.
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

/**
 * @brief Report the value of Intellisat's millisecond counter.
 *
 * This will be used to calculate change in time for control loops.
 *
 * @param curr_millis Return-by-reference pointer.
 *
 * @return vi_get_curr_millis_status A return code.
 */
vi_get_curr_millis_status
vi_get_curr_millis(
    uint64_t *curr_millis
);


typedef enum {
    GET_ANGVEL_SUCCESS,
    GET_ANGVEL_FAILURE
} vi_get_angvel_status;

/**
 * @brief Retrieve angular velocity data from the IMU.
 *
 * @param imu Which inertial measurement unit to read from.
 * @param angvel_x,angvel_y,angvel_z Return-by-reference ptrs.
 *
 * The sign of the angular velocity values must adhere to the
 *   Right-Hand-Rule as defined by the satellite's positive axes.
 *
 * @return vi_get_angvel_status A return code.
 */
vi_get_angvel_status
vi_get_angvel(
    vi_IMU_choice imu,
    double *angvel_x, 
    double *angvel_y,
    double *angvel_z
);


typedef enum {
	VI_GET_MAG_SUCCESS,
	VI_GET_MAG_FAILURE
} vi_get_mag_status;

/**
 * @brief Get the current magnetic field value.
 *
 * @param mag Which magnetometer to read from.
 * @param mag_x,mag_y,mag_z The magnetic field vector.
 *
 * @return vi_get_mag_status A return code, success/failure.
 */
vi_get_mag_status
vi_get_mag(
    vi_MAG_choice mag,
	double *mag_x,
	double *mag_y,
	double *mag_z
);


typedef enum {
    VI_GET_CSS_SUCCESS,
    VI_GET_CSS_FAILURE
} vi_get_css_status;

/**
 * @brief Get a coarse sun sensor (CSS) reading.
 *
 * @param css Which CSS to read from.
 * @param magnitude Return-by-reference pointer.
 *
 * The CSS' magnitude would ideally in the range [0, 1]
 *  with 1 corresponding to the sun being directly overhead,
 *  but users should expect the output of this function to
 *  deviations both above and below this range.
 *
 * @return vi_get_css_status A return code, success/failure.
 */
vi_get_css_status
vi_get_css(
    vi_CSS_choice css,
    double *magnitude
);

typedef enum {
    VI_GET_TEMP_SUCCESS,
    VI_GET_TEMP_FAILURE
} vi_get_temp_status;

/**
 * @brief Get a temperature sensor reading.
 *
 * @param sensor Which temperature sensor to read from.
 * @param temp Return-by-reference pointer.
 *
 * @return vi_get_temp_status A return code, success/failure.
 */
vi_get_temp_status 
vi_get_temp(
	vi_tmp_choice sensor, 
	double* temp
);


typedef enum {
    VI_GET_COILS_CURRENT_SUCCESS,
    VI_GET_COILS_CURRENT_FAILURE
} vi_get_coils_current_status;

/**
 * @brief Get a coils current reading.
 *
 * @param currentX, currentY, currentZ Return-by-reference pointer for each 
 * 		  of the coils' current
 *
 * @return vi_get_coils_current_status A return code, success/failure.
 */
vi_get_coils_current_status vi_get_coils_current(
	double* currentX,
	double* currentY, 
	double* currentZ
);


typedef enum {
    VI_GET_SOLAR_PANEL_CURRENT_SUCCESS,
    VI_GET_SOLAR_PANEL_CURRENT_FAILURE
} vi_get_solar_panel_current_status;

/**
 * @brief Get a solar panel current reading.
 *
 * @param sp Which solar panel to read from.
 * @param current Return-by-reference pointer.
 *
 * @return vi_get_solar_panel_current_status A return code, success/failure.
 */
vi_get_solar_panel_current_status
vi_get_solar_panel_current(
	vi_sol_choice sp,
	double* current
);

/*###################### CONSTANTS ######################*/

typedef enum{
	GET_CONSTANT_SUCCESS = 0,
	GET_CONSTANT_FAILURE = 1
} vi_get_constant_status;

// Note: I really really want to make the offset, scalar & constant into struct
typedef struct{
	float offset;
	float scalar;
	float filter_constant;
}  sensorCal;

/**@brief Get the current calibration values for a sensor.
 *
 * @param sensor we want calibration for.
 * @param offset,scalar Return-by-reference ptrs.
 * @param filter_constant An attenuation constant between
 *  0.0 - 1.0 for lowpass filter calculation. A greater
 *  value causes more damping on large jumps in sensor data.
 *
 * @return vi_get_constant_status A return code, success/failure.
 */
vi_get_constant_status vi_get_sensor_calibration(
	vi_sensor sensor, 
	float *offset,
	float *scalar,
	float *filter_constant
);


/**@brief Get the current status (on/off) of a given sensor.
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


/**@brief Get the experiment generation.
 *
 * @return The generation as an int.
 */
int vi_get_experiment_generation();


/**@brief Increment the experiment generation.
 *
 * @return Void.
 */
void vi_increment_experiment_generation();


/**@brief Get the detumbling generation.
 *
 * @return The generation as an int.
 */
int vi_get_detumbling_generation();


/**@brief Increment the detumbling generation.
 *
 * @return Void.
 */
void vi_increment_detumbling_generation();

/**@brief Get the detumbling generation.
 *
 * @return The generation as an int.
 */
int vi_get_determination_generation();


/**@brief Increment the detumbling generation.
 *
 * @return Void.
 */
void vi_increment_determination_generation();


/*###################### OPERATIONS ######################*/

typedef enum {
    VI_DELAY_MS_SUCCESS,
    VI_DELAY_MS_FAILURE
} vi_delay_ms_status;

/**@brief Sleep for some number of milliseconds.
 *
 * @param ms The number of milliseconds to sleep for.
 *
 * @return vi_delay_ms_status A return code, success/failure.
 */
vi_delay_ms_status
vi_delay_ms(
    int ms
);


/**@brief Print a string.
 *
 * @param string The string to print.
 *
 * @return Void.
 */
void 
vi_print (
  const char *message, ...
);



/**@brief Configure the data logger for a particular mode.
 *
 * @param mode The logger mode setting.
 *
 * @return Void.
 */
void
vi_configure_logging_mode(
    adcs_mode mode
);


#endif//VIRTUAL_INTELLISAT_H
