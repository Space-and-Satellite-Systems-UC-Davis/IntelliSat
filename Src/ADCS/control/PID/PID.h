/**@file PID.h
 *
 * @brief Implementation of a PID (proportional–integral–derivative) controller.
 *
 * @author Charles Kvoriak (charles.kvoriak@gmail.com) 9/25/2024
 */

#ifndef PID_H
#define PID_H

#include <stdint.h>

typedef struct {
	double P_gain;
	double I_gain;
	double D_gain;
	double e_prev;
	double e_cumulative;
	uint64_t t_prev;
} PID_controller;

/**@brief Generate an actuator command.
 *
 * @param target The current target for the system. May be changed over time.
 * @param state The current state of the system.
 * @param t_curr The current time in system-milliseconds. Not absolute.
 * @param controller Pointer to the PID_controller struct.
 *
 * @return A command to the actuator.
 */
double PID_command(
	double target,
	double state,
	uint64_t t_curr,
	PID_controller *controller
);

/**@brief Initialize a new PID_controller with appropriate values.
 *
 * @param target_init The initial target for the system.
 * @param state_init The initial state of the system.
 * @param t_init The initial time in system-milliseconds. Not absolute.
 * @param P_gain The gain for the proportional term of the controller.
 * @param I_gain The gain for the integral term of the controller.
 * @param D_gain The gain for the derivative term of the controller.
 * @param controller Pointer to the PID_controller struct.
 *
 * @return A command to the actuator.
 */
void PID_init(
	double target_init,
	double state_init,
	uint64_t t_init,
	double P_gain,
	double I_gain,
	double D_gain,
	PID_controller *controller
);

#endif //PID_H
