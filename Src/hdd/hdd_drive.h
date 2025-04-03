/*
 * Any functions or code relevant to directly driving the
 * rotation of the hdd
 */

#include <Timers/timers.h>
#include <Timers/system_timer.c>
#include <ADC/adc.h>
#include <LED/led.h>
#include <inttypes.h>

#pragma once

// default ramp
void ramp(const float TARGET_DUTY, const float MIN_DUTY, const float MAX_DUTY);

// reverse ramp
void rampRev(const float TARGET_DUTY, const float MIN_DUTY, const float MAX_DUTY);

// sets the duty below the minimum the esc will accept through toggling of power
void setSubMinDuty(const float PERCENT, const float MIN_DUTY, const float ZERO_DUTY, const uint64_t TIME_MS);

void setSubMinDuty(const float PERCENT, const float MIN_DUTY, const float ZERO_DUTY, const uint64_t TIME_MS) {
	if (PERCENT >= 1) {
		pwm_setDutyCycle(MIN_DUTY);
		delay_ms(TIME_MS);
		return;
	}

	uint64_t end_time = getSysTime() + TIME_MS;

	// to retain smooth behavior, period must decrease as percent decreases
	// since there will be much more of the period to lose speed on
	// 125ms off time is a fairly smooth, so we adjust the period to reach this
	const uint64_t ON_TIME = PERCENT * 125 / (1 - PERCENT);
	while(getSysTime() < end_time) {
		pwm_setDutyCycle(MIN_DUTY);
		delay_ms(ON_TIME);

		pwm_setDutyCycle(ZERO_DUTY);
		delay_ms(125);
	}
}

void ramp(const float TARGET_DUTY, const float MIN_DUTY, const float MAX_DUTY){
	printMsg("Starting Execution. \r\n");
	const float MID_DUTY = (MAX_DUTY + MIN_DUTY) / 2;
	const float DUTY_STEP = (MAX_DUTY - MID_DUTY) / 10;
	const int DELAY_MS = 250;  // time between ramps

	// start at the middle duty + some base speed, then slowly increase
	const float INIT_DUTY = MID_DUTY + DUTY_STEP;
	float current_duty = INIT_DUTY;
	while (current_duty <= TARGET_DUTY && current_duty <= MAX_DUTY){
		pwm_setDutyCycle(current_duty);
		printMsg("Duty cycle: %f \r\n", current_duty);
		current_duty += DUTY_STEP;
		delay_ms(DELAY_MS);
	}
}

void rampRev(const float TARGET_DUTY, const float MIN_DUTY, const float MAX_DUTY) {
	ramp(TARGET_DUTY, MAX_DUTY, MIN_DUTY);
}
