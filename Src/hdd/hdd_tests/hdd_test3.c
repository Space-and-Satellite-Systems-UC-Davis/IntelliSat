/*
 * hdd_training_test.c
 *
 *  Fill out the information below
 *  Created on: April 2, 2025
 *  Updated on: April 2, 2025
 *      Author: Oseghale Oboh
 *
 *  Information:
 *  	This file is the main hdd test function file and should
 *  	not be changed by anyone other than the sub-lead/lead, unless
 *  	the go-ahead has been given for other members.
 */

/*
 -----------vars-----------
MAX_START_DUTY - raw percentage
PERIOD_US - period in micro second (we have no mew symbol)


	MIN_DUTY - minimum range of PWM signal without
	MAX_DUTY - maximum range of PWM signal
				this is the range the PWM signal's "high time" must be in for the esc Electronic Speed Controller
				 all ranges must be within 1 & 2(scaled proportionally to the PERIOD_US)

MID_DUTY - middle of range duty
DUTY_STEP - how much the duty will increment
DRIVE_DUTY - target duty value
ZERO_DUTY - the PWM value that indicates no speed PWM
 -----------vars-----------

----------------- pseudo ------------------
includes

main / testing function
	set vars
	do initialization
		plug set vars into init function(s)
		set mid, zero, target, step duty

	calibrate or arm

	ramp function

	debug ramp function if selected

	end prog by calling PWM timer off
 ----------------- pseudo ------------------
 */

#include <Timers/timers.h>
#include <ADC/adc.h>
#include <LED/led.h>
#include <inttypes.h>

//--------------- includes -----------------
#include "../hdd_drive.h"
#include "../hdd_init.h"
//--------------- includes -----------------

//===================================================================================
//================================ main test function ================================
//===================================================================================
// runs the actual testing code
void testFunction_HDD3() {

	//------------------- var inits part 1----------------------
	const int PERIOD_uS = 20000;  //uS means mew seconds (microseconds) (5% duty -> min (1ms pulse), 10% duty -> max (2ms pulse)),,, 20ms
	const float MAX_START_DUTY = 10;  // previous max duty to trigger calibration
	const float MAX_DUTY = (2 * 1000 * 100) / PERIOD_uS;  // targeted current max duty (should be no higher than 10 for 2ms pulses)
	const float MIN_DUTY = (1 * 1000 * 100) / PERIOD_uS;  // targeted current min duty (should be no lower than 5 for 1ms pulses)
	//------------------- var inits part 1----------------------


	//--------------------- initialization functions ---------------
	// init timer here does other initialization actions
	led_d2(1);
	pwm_initTimer(PERIOD_uS); //This period time is in microseconds //starts clock for pwm cycle
	pwm_setDutyCycle(10);
	PWM_TIMER_ON();

	adc_init();
	printMsg("Initializing ADC\r\n");

	adc_enable();
	printMsg("Enabling ADC\r\n");

	adc_configGPIO();
	printMsg("Configuring GPIO Pin A2 for ADC\r\n");

	//Right now it sets it pin A2, which is photodiode/sunsensor 0 (SOL-0)
	//Plug in TP2 to the bottommost wire on the connector and TP1 to the one above it
	adc_setChannel();
	printMsg("ADC1 channel 7, GPIO Pin A2 set\r\nBeginning to read values\r\n");
	//--------------------- initialization functions ---------------


	//------------------- var inits part 2----------------------
	const float MID_DUTY = (MAX_DUTY + MIN_DUTY) / 2;
	const float DUTY_STEP = 0.5;
	const float DRIVE_DUTY = MAX_DUTY;
	//------------------- var inits part 2----------------------


	//-------------------------------- calibration or arming ------------------------------
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
	//-------------------------------- calibration or arming ------------------------------


	//-------------------------------- simple tests ----------------------------
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
	//-------------------------------- simple tests ----------------------------


	//------------------------------ testing loop ----------------------------
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
	//------------------------------ testing loop ----------------------------


	//--------------------------- end prog -------------------
	printMsg("Ending program. \r\n");
	PWM_TIMER_OFF();
	//--------------------------- end prog -------------------
}
//===================================================================================
//================================ main test function ================================
//===================================================================================
