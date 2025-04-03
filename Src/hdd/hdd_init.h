/*
 * For functions necessary to initialize the esc or
 * hdd prior to activation
 */

#include <Timers/timers.h>
#include <ADC/adc.h>
#include <LED/led.h>
#include <inttypes.h>

#pragma once

// if max should be calibrated, the min and max duty, and the max duty previous calibrated
void calibrate(const bool CAL_MAX, const float MIN_DUTY, const float MAX_DUTY, const float MAX_START_DUTY);

// it seems like to arm, the esc just expects an absolute min duty for 0 after getting some signal
void arm(const float MIN_DUTY, const float MAX_DUTY);

void calibrate(const bool CAL_MAX, const float MIN_DUTY, const float MAX_DUTY, const float MAX_START_DUTY) {
	pwm_setDutyCycle(MAX_START_DUTY); // trigger calibration
	printMsg("Min duty: %f, Max duty: %f, Max start duty: %f \r\n", MIN_DUTY, MAX_DUTY, MAX_START_DUTY);
	delay_ms(1000);

	if (CAL_MAX) {
		printMsg("Feeding Maximum duty. \r\n");
		pwm_setDutyCycle(MAX_DUTY);
		delay_ms(4000);
	} else {
		printMsg("Feeding minimum duty. \r\n");
		pwm_setDutyCycle(MIN_DUTY);
		delay_ms(4000);
	}
}

void arm(const float MIN_DUTY, const float MAX_DUTY) {
	const float MID_DUTY = (MAX_DUTY + MIN_DUTY) / 2;
	const float DUTY_STEP = (MAX_DUTY - MID_DUTY) / 8;
	float currDuty = MID_DUTY;

	printMsg("Arming with the following parameters: \n");
	printMsg("Min duty: %f, Max duty: %f, Zero duty: %f, Duty step: %f \r\n", MIN_DUTY, MAX_DUTY, MID_DUTY, DUTY_STEP);

	// The ESC needs to detect some signal, then to drop to minimum
	// the zero/mid duty is just half way between max and minimum,
	// so it should count as some signal. From here, we ramp down to 0
	currDuty -= DUTY_STEP; // go below max
	printMsg("Ramping down. \r\n");
	while (currDuty >= MIN_DUTY) {
		pwm_setDutyCycle(currDuty);
		printMsg("Current duty: %f \r\n", currDuty);
		delay_ms(250);
		currDuty -= DUTY_STEP;
	}

	printMsg("Return to zero. \r\n");
	pwm_setDutyCycle(MID_DUTY);
}
