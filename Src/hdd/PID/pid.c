#include "PID.h"

double PID_command(
	double target,
	double state,
	int t_curr,
	PID_controller *controller
) {
	double e_curr = target - state;
	double dt = get_delta_t(t_curr, (controller->t_prev));
	double e_derivative = (e_curr - controller->e_prev)/dt;

	double command = (controller->P_gain)*e_curr
				   + (controller->I_gain)*(controller->e_cumulative)
				   + (controller->D_gain)*e_derivative;

	controller->e_cumulative = controller->e_cumulative + dt*e_curr;
	controller->e_prev = e_curr;

	return command;
}

void PID_init(
	double target_init,
	double state_init,
	int t_init,
	double P_gain,
	double I_gain,
	double D_gain,
	PID_controller *controller
) {
	controller->P_gain = P_gain;
	controller->I_gain = I_gain;
	controller->D_gain = D_gain;
	controller->e_prev = state_init - target_init;
	controller->t_prev = t_init;
	controller->e_cumulative = 0.0;
}
