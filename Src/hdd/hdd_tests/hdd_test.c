/*
 * hdd_training_test.c
 *
 *  Created on: Sep 25, 2024
 *  Updated on: Feb 18, 2025
 *      Authors: Chandrark Muddana
 *      		 Nicholas Bianez
 *
 *  Information:
 *  	This file is the main hdd test function file and should
 *  	not be changed by anyone other than the sub-lead/lead, unless
 *  	the go-ahead has been given for other members.
 */

#include "../hdd_drive.h"
#include "../hdd_init.h"

#include <Timers/timers.h>
#include <ADC/adc.h>
#include <LED/led.h>
#include <inttypes.h>

// the member id to use for test function execution
int testFuncID_HDD() {
	// return your id value to call your test function
	return 2;
}

// runs the actual testing code
void testFunction_HDD() {
	const float MAX_START_DUTY = 10;  // previous max duty to trigger calibration
	const float MAX_DUTY = 10;  // targeted current max duty (should be no higher than 10 for 2ms pulses)
	const float MIN_DUTY = 5;  // targeted current min duty (should be no lower than 5 for 1ms pulses)
	const int PERIOD_uS = 20000;  // period is microseconds (5% duty -> min (1ms pulse), 10% duty -> max (2ms pulse))

	// init timer here does other initialization actions
	led_d2(1);
	pwm_initTimer(PERIOD_uS); //This period time is in microseconds
	pwm_setDutyCycle(10);
	PWM_TIMER_ON();

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
		arm(MIN_DUTY, MAX_DUTY);
	}

	printMsg("Starting Execution. \r\n");
	pwm_setDutyCycle(100);
	//ramp(DRIVE_DUTY, MIN_DUTY, MAX_DUTY);
	delay_ms(1000);

	printMsg("Testing minimum duty");
	pwm_setDutyCycle(MIN_DUTY);
	delay_ms(5000);

	printMsg("Testing mid duty");
	pwm_setDutyCycle(MID_DUTY);
	delay_ms(5000);

	printMsg("Testing max duty");
	pwm_setDutyCycle(MAX_DUTY);
	delay_ms(5000);

	// debug loop
	while (0) {
		// rapidly increase until max is hit
		printMsg("Preparing to start trial. \r\n");
		float currDuty = MIN_DUTY;
		while (currDuty < MID_DUTY) {
			pwm_setDutyCycle(currDuty);
			printMsg("Duty: %f \r\n", currDuty);
			delay_ms(500);
			currDuty += DUTY_STEP;
		}

		printMsg("Waiting to up duty cycle. \r\n");
		delay_ms(5000);
		while (currDuty <= MAX_DUTY) {
			pwm_setDutyCycle(currDuty);
			printMsg("Duty: %f \r\n", currDuty);
			delay_ms(500);
			currDuty += DUTY_STEP;
		}

		delay_ms(5000);

		break;
	}

	printMsg("Ending program. \r\n");
	PWM_TIMER_OFF();
}
