/*@file virtual_intellisat.h
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
 *  which vastly simplifies development for everyone. And, while
 *  such code will not run, it will still compile!
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


#endif//VIRTUAL_INTELLISAT_H





