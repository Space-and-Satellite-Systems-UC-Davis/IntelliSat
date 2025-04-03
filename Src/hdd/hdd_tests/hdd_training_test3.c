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
MAX_START_DUTY - percentage
PERIOD_US - period in micro second (we have no mew symbol)

		this is the range the PWM signal's "high time" must be in for the esc Electronic Speed Controller
				 all ranges must be within 1 & 2(scaled proportionality to the PERIOD_US)
	MIN_DUTY - minimum range of PWM signal without ?breaking? the hdd spin?
	MAX_DUTY - maximum range of PWM signal

MID_DUTY - middle of range duty
DUTY_STEP - how much the duty will increment
DRIVE_DUTY - target duty value
ZERO_DUTY - the PWM value that indicates no speed PWM


 -----------vars-----------

 ----------------- psedu ------------------
includes
proto funcs

main / testing function
	set vars
	do initialization
		plug set vars into init function(s)
		set mid, zero, target, step duty

	calibrate or arm

	ramp function

	debug ramp function if selected

	end prog by calling PWM timer off
 ----------------- psedu ------------------

 --------------questiosn?--------------
what is duty
what is duty step
initialization functions
setdutycycle(10) = 20% ??


 */


//--------------- includes -----------------
#include "hdd_init.c"
#include "hdd_drive.c"
//--------------- includes -----------------


//-------------- defunct function ?? ---------------------
// the member id to use for test function execution
void testFuncID_HDD() {
	// return your id value to call your test function
	return 0;
}
//-------------- defunct function ?? ---------------------


//===================================================================================
//================================ main test function ================================
//===================================================================================
// runs the actual testing code
void testFunctionMain_HDD3(){

	//------------------- var inits part 1----------------------
	const float MAX_START_DUTY = 10;  // previous max duty to trigger calibration
	const float MAX_DUTY = 10;  // targeted current max duty (should be no higher than 10 for 2ms pulses)
	const float MIN_DUTY = 5;  // targeted current min duty (should be no lower than 5 for 1ms pulses)
	const int PERIOD_uS = 20000;  // period is microseconds (5% duty -> min (1ms pulse), 10% duty -> max (2ms pulse))
	//------------------- var inits part 1----------------------

	//--------------------- initialization functions ---------------
	// init timer here does other initialization actions
	led_d2(true);
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

	//-------------------------------- calibration or arm ------------------------------
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
	//-------------------------------- calibration or arm ------------------------------


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
