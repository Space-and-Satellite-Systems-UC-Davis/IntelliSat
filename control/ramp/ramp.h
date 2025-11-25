/**@file ramp.h
 *
 * @brief Interface to ramp controller.
 *
 * @author Jacob Tkeio (jacobtkeio@gmail.com) 10/25/2024
 */

#ifndef RAMP_H
#define RAMP_H


/* Create a ramp_controller struct to store ramp parameters.
 *  Initialize the parameters with init_ramp_controller.
 *  Generate ramp commands with it in the function(s) below.
 */
typedef struct {
    unsigned int starting_ms;
    unsigned int left_leg_length_ms;
    unsigned int right_leg_length_ms;
    unsigned int plateau_length_ms;
    double plateau_height;
} ramp_controller;


/**@brief Initialize a ramp_controller struct with ramp parameters.
 *
 * @param starting_ms When the ramp starts [ms]. Can be in the future.
 *                    No command will be output until starting_ms.
 * @param left_leg_length_ms The duration of the rising ramp [ms]
 * @param right_leg_length_ms The duration of the falling ramp [ms]
 * @param plateau_height The height of the ramp's plateau in [UNIT].
 *                       Can be negative.
 * @param plateau_length_ms The length of the ramp's plateau in [ms].
 *
 * @return Void.
 */
void init_ramp_controller(
    unsigned int starting_ms,
    unsigned int left_leg_length_ms,
    unsigned int right_leg_length_ms,
    unsigned int plateau_length_ms,
    double plateau_height,
    ramp_controller *controller
);


/**@brief Generate a linear ramp command for the actuator.
 *
 * @param t_curr The current time in milliseconds.
 * @param controller Pointer to ramp parameter struct.
 *
 * @return An actuator command [UNIT based on init_ramp_controller].
 */
double linear_ramp_command (
    unsigned int t_curr,
    ramp_controller *controller
);


#endif //RAMP_H
