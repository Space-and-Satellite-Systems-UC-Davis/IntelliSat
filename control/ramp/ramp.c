/**@file ramp.c
 *
 * @brief Implementation of ramp controller.
 *
 * @author Jacob Tkeio (jacobtkeio@gmail.com) 10/25/2024
 */

#include "ramp.h"

double linear_ramp_command (
    unsigned int t_curr,
    ramp_controller *c
) {
    if (c->starting_ms + c->left_leg_length_ms + c->plateau_length_ms + c->right_leg_length_ms < 0) {
        return 0.0;
    }
    unsigned int region_ms;
    region_ms  = c->starting_ms;
    if (t_curr < region_ms)
        return 0.0;
    region_ms += c->left_leg_length_ms;
    if (t_curr < region_ms)
        return (t_curr - c->starting_ms)
             * (c->plateau_height / c->left_leg_length_ms);
    region_ms += c->plateau_length_ms;
    if (t_curr < region_ms)
        return c->plateau_height;
    region_ms += c->right_leg_length_ms;
    if (t_curr < region_ms)
        return c->plateau_height
            - (t_curr - c->starting_ms
                      - c->left_leg_length_ms
                      - c->plateau_length_ms)
            * (c->plateau_height / c->right_leg_length_ms);
    return 0.0; //else
}

void init_ramp_controller(
    unsigned int starting_ms,
    unsigned int left_leg_length_ms,
    unsigned int right_leg_length_ms,
    unsigned int plateau_length_ms,
    double plateau_height,
    ramp_controller *controller
) {
    controller->starting_ms         =  starting_ms;
    controller->left_leg_length_ms  =  left_leg_length_ms;
    controller->right_leg_length_ms =  right_leg_length_ms;
    controller->plateau_length_ms   =  plateau_length_ms;
    controller->plateau_height      =  plateau_height;
}
