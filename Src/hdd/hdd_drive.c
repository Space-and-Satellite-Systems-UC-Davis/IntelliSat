/*
 * hdd_drive.c
 *
 *  Created on: Apr 2, 2025
 *      Author: nmain
 */

#include "hdd_drive.h"
#include "../system_config/Timers/timers.h"

void hddSetSubMinDuty(const float PERCENT, const float MIN_DUTY, const float ZERO_DUTY, const uint64_t TIME_MS) {
	if (PERCENT >= 1) {
		pwm_setDutyCycle(PWM0, MIN_DUTY);
		delay_ms(TIME_MS);
		return;
	}

	uint64_t end_time = getSysTime() + TIME_MS;

	// to retain smooth behavior, period must decrease as percent decreases
	// since there will be much more of the period to lose speed on
	// 125ms off time is a fairly smooth, so we adjust the period to reach this
	const uint64_t ON_TIME = PERCENT * 125 / (1 - PERCENT);
	while(getSysTime() < end_time) {
		pwm_setDutyCycle(PWM0, MIN_DUTY);
		delay_ms(ON_TIME);

		pwm_setDutyCycle(PWM0, ZERO_DUTY);
		delay_ms(125);
	}
}

void hddRamp(const PWM_Channels pwm, const uint8_t TARGET_DUTY, int8_t doPrint) {
	if (doPrint) { printMsg("Ramping to %u. \r\n", TARGET_DUTY); }
	const uint8_t DUTY_STEP = 1;
	const uint32_t DELAY_MS = 50;  // time between ramps

	// start at the middle duty + some base speed, then slowly increase
	uint8_t currDuty = pwm_getDutyCycle(pwm) + DUTY_STEP;
	if (doPrint) { printMsg("Taking first duty step to %u. \r\n", currDuty); }
	while (currDuty <= TARGET_DUTY && currDuty <= MAX_DUTY){
		// set the duty
		pwm_setDutyCycle(pwm, currDuty);
		//if (doPrint) { printMsg("Duty cycle: %u \r\n", currDuty); }

		// exit if we just set pwm to the target, otherwise increment
		if (currDuty == TARGET_DUTY) { break; }
		currDuty += DUTY_STEP;

		// make sure a loop with currDuty == TARGET_DUTY always happens
		if (currDuty > TARGET_DUTY) { currDuty = TARGET_DUTY; }
		delay_ms(DELAY_MS);
	}

	if (doPrint) { printMsg("Ramped to %u. \r\n", TARGET_DUTY); }
}

void hddDrive(const PWM_Channels pwm, const uint8_t TARGET_DUTY, int8_t doPrint) {
	if (TARGET_DUTY <= SLIP_DUTY) {
		if (doPrint) { printMsg("Drove to %u without slip handling.\r\n", TARGET_DUTY); }
		pwm_setDutyCycle(pwm, TARGET_DUTY);
		return;
	}

	// if we would slip going straight to target, ramp instead
	if (doPrint) { printMsg("Driving to %u with slip handling.\r\n", TARGET_DUTY); }
	pwm_setDutyCycle(pwm, SLIP_DUTY);
	uint8_t currDuty = pwm_getDutyCycle(pwm);

	// wait for speed up, then begin ramp
	uint32_t delayTimeMs = currDuty <= MID_DUTY ? SLIP_TIME_MS : JUMP_TIME_MS;
	currDuty = pwm_getDutyCycle(pwm);
	if (doPrint) { printMsg("Delaying for %u with currDuty %u\r\n", delayTimeMs, currDuty); }
	delay_ms(delayTimeMs);
	//pwm_setDutyCycle(pwm, TARGET_DUTY);  // try to make next complete jump
	hddRamp(pwm, TARGET_DUTY, doPrint);
}

void hddDriveNB(const PWM_Channels pwm, dutyType TARGET_DUTY) {
	// limit the targeted duty if we would slip trying to immediately jump
	if (TARGET_DUTY > SLIP_DUTY && pwm_getDutyCycle(pwm) < SLIP_DUTY) {
		pwm_setDutyCycle(pwm, SLIP_DUTY);
		delay_ms(50);
		hddRamp(pwm, TARGET_DUTY, 0);
		return;
	}

	pwm_setDutyCycle(pwm, TARGET_DUTY);
}

void hddRampRev(const float TARGET_DUTY, const float MIN_DUTY, const float MAX_DUTY) {
	//ramp(TARGET_DUTY, MAX_DUTY, MIN_DUTY);
}
