/*
 * hdd_init.c
 *
 *  Created on: Apr 2, 2025
 *      Author: nmain
 */

#include "hdd_init.h"
#include "../system_config/Timers/timers.h"

const dutyType MAX_START_DUTY = 100;  // previous max duty to trigger calibration
const dutyType SLIP_DUTY = 75;  // last valid duty that will not cause any slipping
const dutyType MAX_DUTY = 100;  // targeted current max duty (should be no higher than 10 for 2ms pulses)
const dutyType MID_DUTY = 50;
const dutyType MIN_DUTY = 0;  // targeted current min duty (should be no lower than 5 for 1ms pulses)
const uint32_t SLIP_TIME_MS = 750;  // safe amount of time to wait to go from mid to slip duty
const uint32_t JUMP_TIME_MS = 250;  // safe amount of time to wait to go from >mid to <= slip duty
const int PERIOD_uS = 2000;  // period is microseconds (5% duty -> min (1ms pulse), 10% duty -> max (2ms pulse))

void hdd_init(const PWM_Channels channel){
	pwm_initTimer(channel, PERIOD_uS);
	pwm_setDutyCycle(channel, 100);
	pwm_timerOn(channel);
}

void hdd_calibrate(const PWM_Channels channel, const int CAL_MAX) {
	pwm_setDutyCycle(channel, MAX_START_DUTY); // trigger calibration
	printMsg("Min duty: %f, Max duty: %f, Max start duty: %f \r\n", MIN_DUTY, MAX_DUTY, MAX_START_DUTY);
	delay_ms(1000);

	if (CAL_MAX) {
		printMsg("Feeding Maximum duty. \r\n");
		pwm_setDutyCycle(channel, MAX_DUTY);
		delay_ms(4000);
	} else {
		printMsg("Feeding minimum duty. \r\n");
		pwm_setDutyCycle(channel, MIN_DUTY);
		delay_ms(4000);
	}
}

void hdd_arm(PWM_Channels channel) {
	const float DUTY_STEP = (MAX_DUTY - MID_DUTY) / 8;
	float currDuty = MID_DUTY;

	printMsg("Arming with the following parameters: \r\n");
	printMsg("Min duty: %f, Max duty: %f, Zero duty: %f, Duty step: %f \r\n", MIN_DUTY, MAX_DUTY, MID_DUTY, DUTY_STEP);

	// The ESC needs to detect some signal, then to drop to minimum
	// the zero/mid duty is just half way between max and minimum,
	// so it should count as some signal. From here, we ramp down to 0
	printMsg("Ramping down. \r\n");
	while (currDuty >= MIN_DUTY) {
		pwm_setDutyCycle(channel, currDuty);
		printMsg("Current duty: %f \r\n", currDuty);
		delay_ms(250);
		currDuty -= DUTY_STEP;
	}

	printMsg("Return to zero. \r\n");
	pwm_setDutyCycle(channel, MID_DUTY);

	delay_ms(5000);
}
