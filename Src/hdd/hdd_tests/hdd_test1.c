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

	// init timer here does other initialization actions
	const int targetPWM = PWM1;
	led_d2(1);
	led_d3(0);
	hdd_init(targetPWM);
	//pwm_initTimer(PWM0, PERIOD_uS); //This period time is in microseconds
	//pwm_setDutyCycle(PWM0, 10);
	//pwm_timerOn(PWM0);

	const float DUTY_STEP = 0.5;

	// calibrate or arm
	if (0) {
		// calibration controls what PWM duty cycle the
		// ESC considers as the maximums and minimums
		printMsg("Calibrating max duty. \r\n");
		hdd_calibrate(targetPWM, 1);

		printMsg("Calibrating min duty in 3 seconds. \r\n");
		delay_ms(3000);
		hdd_calibrate(targetPWM, 0);

		printMsg("Calibration completed; 3 seconds until calibration concludes \r\n.");
		delay_ms(3000);
		printMsg("Continuing. \r\n");
	}

	hdd_arm(targetPWM);
	printMsg("Testing slipping.\r\n");
	pwm_setDutyCycle(targetPWM, SLIP_DUTY);
	delay_ms(5000);
	pwm_setDutyCycle(targetPWM, MID_DUTY);
	delay_ms(5000);

	led_d2(0);
	led_d3(1);
	printMsg("Testing driving.\r\n");
	hddDrive(targetPWM, 95, 1);
	uint8_t duty = pwm_getDutyCycle(targetPWM);
	printMsg("Set duty to %u and got duty %u\r\n", 95, duty);
	delay_ms(10000);

	led_d2(1);
	led_d3(1);
	printMsg("Starting Execution. \r\n");
	pwm_setDutyCycle(targetPWM, 100);
	//ramp(DRIVE_DUTY, MIN_DUTY, MAX_DUTY);
	delay_ms(1000);

	printMsg("Testing minimum duty \r\n");
	pwm_setDutyCycle(targetPWM, MIN_DUTY);
	delay_ms(5000);

	printMsg("Testing mid duty \r\n");
	pwm_setDutyCycle(targetPWM, MID_DUTY);
	delay_ms(5000);

	printMsg("Testing max duty \r\n");
	pwm_setDutyCycle(targetPWM, MAX_DUTY);
	delay_ms(5000);

	led_d2(0);
	led_d3(1);
	// debug loop
	while (0) {
		// rapidly increase until max is hit
		printMsg("Preparing to start trial. \r\n");
		float currDuty = MIN_DUTY;
		while (currDuty < MID_DUTY) {
			pwm_setDutyCycle(targetPWM, currDuty);
			printMsg("Duty: %f \r\n", currDuty);
			delay_ms(500);
			currDuty += DUTY_STEP;
		}

		printMsg("Waiting to up duty cycle. \r\n");
		delay_ms(5000);
		while (currDuty <= MAX_DUTY) {
			pwm_setDutyCycle(targetPWM, currDuty);
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
	pwm_timerOff(targetPWM);
}
