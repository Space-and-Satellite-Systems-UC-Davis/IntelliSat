#include "timers.h"

void branch_main() {
	init_pwm_timer(100);
	set_duty_cycle(20);

	PWM_TIMER_ON();

	while (1) {
	}
}
