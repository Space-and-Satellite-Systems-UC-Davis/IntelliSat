/*
 * hdd_init.c
 *
 *  Created on: Apr 2, 2025
 *      Author: nmain
 */

#include <Timers/timers.h>
#include <ADC/adc.h>
#include <LED/led.h>
#include <inttypes.h>
#include <stdbool.h>

const float MAX_START_DUTY = 10;  // previous max duty to trigger calibration
const float MAX_DUTY = 10;  // targeted current max duty (should be no higher than 10 for 2ms pulses)
const float MIN_DUTY = 5;  // targeted current min duty (should be no lower than 5 for 1ms pulses)
const int PERIOD_uS = 20000;  // period is microseconds (5% duty -> min (1ms pulse), 10% duty -> max (2ms pulse))

void hdd_init(const PWM_Channels channel){
	pwm_initTimer(channel, PERIOD_uS);
	pwm_setDutyCycle(channel, 100);
	pwm_timerOn(channel);
}

void hdd_calibrate(const PWM_Channels channel, const bool CAL_MAX) {
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
		pwm_setDutyCycle(channel, currDuty);
		printMsg("Current duty: %f \r\n", currDuty);
		delay_ms(250);
		currDuty -= DUTY_STEP;
	}

	printMsg("Return to zero. \r\n");
	pwm_setDutyCycle(channel, MID_DUTY);

	delay_ms(3000);
	pwm_setDutyCycle(PWM0, 100);
	//ramp(DRIVE_DUTY, MIN_DUTY, MAX_DUTY);
	delay_ms(1000);
}
