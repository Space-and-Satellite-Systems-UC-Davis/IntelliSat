/*
 * hdd_training_test.c
 *
 *  Fill out the information below
 *  Created on: 2025-07-22
 *  Updated on: 2025-07-22
 *      Author: Nicholas Bianez
 *
 *
 *  Information:
 *  	This file is designate for personal testing
 *  	to the active HDD member with ID 1.
 */

#include "../hdd_drive.h"
#include "../hdd_init.h"

#include <Timers/timers.h>
#include <ADC/adc.h>
#include <LED/led.h>
#include <inttypes.h>
#include <print_scan.h>

// runs the actual testing code
void testFunction_HDD1() {
	printMsg("Starting HDD1 test function execution.\r\n");
	const float MAX_START_DUTY = 100;  // previous max duty to trigger calibration
	const float MAX_DUTY = 100;  // targeted current max duty (should be no higher than 10 for 2ms pulses)
	const float MIN_DUTY = 64;  // targeted current min duty (should be no lower than 5 for 1ms pulses)
	const int PERIOD_uS = 2000;  // period is microseconds (5% duty -> min (1ms pulse), 10% duty -> max (2ms pulse))

	// init timer here does other initialization actions
	led_d2(1);
	led_d3(0);
	pwm_initTimer(PWM0, PERIOD_uS); //This period time is in microseconds
	pwm_setDutyCycle(PWM0, 10);
	pwm_timerOn(PWM0);

	const float MID_DUTY = (MAX_DUTY + MIN_DUTY) / 2;
	const float DUTY_STEP = 0.5;
	const float DRIVE_DUTY = MAX_DUTY;

	// calibrate or arm
	if (0) {
		// calibration controls what PWM duty cycle the
		// ESC considers as the maximums and minimums
		printMsg("Calibrating max duty. \r\n");
		calibrate(true, MIN_DUTY, MAX_DUTY, MAX_START_DUTY);

		printMsg("Calibrating min duty in 3 seconds. \r\n");
		delay_ms(3000);
		calibrate(false, MIN_DUTY, MAX_DUTY, MAX_START_DUTY);

		printMsg("Calibration completed; 3 seconds until calibration concludes \r\n.");
		delay_ms(3000);
		printMsg("Continuing. \r\n");
	} else {
//		arm(MIN_DUTY, MAX_DUTY);
	}

	led_d2(1);
	led_d3(1);
	printMsg("Starting Execution. \r\n");
	pwm_setDutyCycle(PWM0, 100);
	//ramp(DRIVE_DUTY, MIN_DUTY, MAX_DUTY);
	delay_ms(1000);

	printMsg("Testing minimum duty \r\n");
	pwm_setDutyCycle(PWM0, MIN_DUTY);
	delay_ms(5000);

	printMsg("Testing mid duty \r\n");
	pwm_setDutyCycle(PWM0, MID_DUTY);
	delay_ms(5000);

	printMsg("Testing max duty \r\n");
	pwm_setDutyCycle(PWM0, MAX_DUTY);
	delay_ms(5000);

	led_d2(0);
	led_d3(1);
	// debug loop
	while (1) {
		// rapidly increase until max is hit
		printMsg("Preparing to start trial. \r\n");
		float currDuty = MIN_DUTY;
		while (currDuty < MID_DUTY) {
			pwm_setDutyCycle(PWM0, currDuty);
			printMsg("Duty: %f \r\n", currDuty);
			delay_ms(500);
			currDuty += DUTY_STEP;
		}

		printMsg("Waiting to up duty cycle. \r\n");
		delay_ms(5000);
		while (currDuty <= MAX_DUTY) {
			pwm_setDutyCycle(PWM0, currDuty);
			printMsg("Duty: %f \r\n", currDuty);
			delay_ms(500);
			currDuty += DUTY_STEP;
		}

		delay_ms(5000);

		break;
	}

	led_d2(0);
	led_d3(0);

	printMsg("Ending program. \r\n");
	pwm_timerOff(PWM0);
}
