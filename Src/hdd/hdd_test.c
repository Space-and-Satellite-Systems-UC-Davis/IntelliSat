/*
 * hdd_training_test.c
 *
 *  Created on: Sep 25, 2024
 *  Updated on: Feb 18, 2025
 *      Authors: Chandrark Muddana
 *      		 Nicholas Bianez
 *
 *  Information:
 *  	This file contains your test code, with
 *  	testFunction_HDD being called when Intellisat is run.
 *  	You can explore how and where this is called by
 *  	right clicking testFunction_HDD, going to declarations,
 *  	then clicking Project. Some functions of interest have
 *  	been included below, with examples of how they work
 *  	and explanations.
 */

#include <Timers/timers.h>
#include <ADC/adc.h>
#include <LED/led.h>
#include <inttypes.h>
#include <print_scan.h>

/*
 * Duty summary:
 * 	Some useful duty values have been provided to you.
 * 	Because the HDD does not spin in reverse, your
 * 	effective duty range is [50, 100]. The duty is
 * 	interpreted as an unsigned byte (0-255),
 * 	corresponding to integer increases in whole
 * 	percentages.
 */
const uint8_t MIN_DUTY = 0;
const uint8_t MID_DUTY = 50;
const uint8_t MAX_DUTY = 100;

// this just sets the speed of your pwm signal;
// do not modify this because it is ESC determined
const uint64_t PERIOD_uS = 2000;

// simple arming function which must be called
// when the ESC is provided power; the ESC
// expects a ramp down from mid duty to 0 duty.
// Ensure this is called after the PWM timer has
// been initialized.
void armESC() {
	// slowly ramp duty down
	uint8_t currDuty = MID_DUTY;
	while (currDuty < 255) {
		pwm_setDutyCycle(PWM0, currDuty);
		currDuty -= 5;
		delay_ms(100);
	}

	delay_ms(3000);  // let ESC finish init
}

// runs the actual testing code
void testFunction_HDD() {

	// init timer here does other initialization actions
	led_d2(1);  // toggle the state of the LED labeled "D2" on the board
	pwm_initTimer(PWM0, PERIOD_uS);  // Initialize the PWM timer to allow PWM signals to work
	pwm_setDutyCycle(PWM0, 50);  //Set the duty to some initial value

	armESC();  // arm the ESC

	printMsg("Starting Execution. \r\n");
	pwm_setDutyCycle(PWM0, 100);
	//ramp(DRIVE_DUTY, MIN_DUTY, MAX_DUTY);
	delay_ms(1000);

	printMsg("Testing max duty");
	pwm_setDutyCycle(PWM0, MAX_DUTY);
	delay_ms(5000);

	printMsg("Testing mid duty");
	pwm_setDutyCycle(PWM0, MID_DUTY);
	delay_ms(5000);

	// rapidly increase until max is hit
	printMsg("Preparing to start trial. \r\n");
	uint8_t currDuty = MID_DUTY;
	while (currDuty < MAX_DUTY) {
		pwm_setDutyCycle(PWM0, ++currDuty);
		printMsg("Duty: %f \r\n", currDuty);
		delay_ms(250);
	}

	printMsg("Ending program. \r\n");
	pwm_timerOff(PWM0);
}
