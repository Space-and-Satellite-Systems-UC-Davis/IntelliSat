/*
 * hdd_training_test.c
 *
 *  Created on: Sep 25, 2024
 *      Author: Chandrark Muddana
 */
#include <Timers/timers.h>
#include <ADC/adc.h>
#include <LED/led.h>
#include <inttypes.h>

// if max should be calibrated, the min and max duty, and the max duty previous calibrated
void calibrate(const bool CAL_MAX, const float MIN_DUTY, const float MAX_DUTY, const float MAX_START_DUTY);

void arm(const float MIN_DUTY, const float MAX_DUTY);

void testFunction_HDD_Training(){
	/*
	 * This is the function where your logic goes.
	 * In IntelliSat we have a testing system so that our main files do not collide when merging
	 * This way we can keep unit tests more organized and integration tests will be more seamless to merge
	 *
	 * We really only care about two things for the training project:
	 * The PWM signal for the ESC.
	 * The ADC for reading the Sun Sensor.
	 *
	 * Our focus will mainly be on the HDD and PWM for now.
	 * The FSW team is responsible for actually generating the PWM signal, so all we have to worry about is driving the motor
	 */
	// This is where write your code.
	// I made some normal code just to generate a PWM signal and check if everything is working.
	led_d1(true);
	pwm_initTimer(20000); //This is in microseconds
	pwm_setDutyCycle(10); //20% of the power
	PWM_TIMER_ON();

	//uint64_t time = getSysTime();
	//printMsg("%llu", getSysTime());
	printMsg( "%d", (int) getSysTime() ) ;
	printMsg("\r\n");
	delay_ms(1000);
	//time = getSysTime();
	printMsg( "%d", (int) getSysTime() ) ;
	printMsg("\r\n");

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
	delay_ms(5000);

	const float MAX_START_DUTY = 20;
	const float MAX_DUTY = 10;
	const float MIN_DUTY = 5;
	const float MID_DUTY = (MAX_DUTY + MIN_DUTY) / 2;
	const float DUTY_STEP = 0.5;

	if (1) {
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

		const float DRIVE_DUTY = MAX_DUTY;
		printMsg("Driving at %f duty. \r\n", DRIVE_DUTY);
		delay_ms(1000);
		pwm_setDutyCycle(DRIVE_DUTY);

		delay_ms(10000);
	}

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

		/*
		 float duty = 7.75f;
		 while (duty < 10.0){
			 delay_ms(125);
			 duty += 0.05;
			 pwm_setDutyCycle(duty);
			 printMsg("%f \r\n", duty);
		 }
		 pwm_setDutyCycle(10);
		 printMsg("This should start spamming. \r\n Working Setup! \r\n");
		 uint16_t adcVal = adc_singleConversion();
		 printMsg("ADC value: %d", adcVal);
		 printMsg(". MilliVolt value: ");
		 adc_printMilliVolt(adc_adcToVolt2(adcVal));
		 printMsg("\r\n");
		 */

		delay_ms(5000);
	}

	printMsg("Ending program. \r\n");
	PWM_TIMER_OFF();
}

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
	const float ZERO_DUTY = (MAX_DUTY + MIN_DUTY) / 2;
	const float DUTY_STEP = (MAX_DUTY - ZERO_DUTY) / 8;
	float currDuty = ZERO_DUTY;

	printMsg("Min duty: %f, Max duty: %f, Zero duty: %f, Duty step: %f \r\n", MIN_DUTY, MAX_DUTY, ZERO_DUTY, DUTY_STEP);
	printMsg("Ramping up. \r\n");
	while (currDuty < MAX_DUTY) {
		pwm_setDutyCycle(currDuty);
		printMsg("Current duty: %f \r\n", currDuty);
		delay_ms(250);
		currDuty += DUTY_STEP;
	}

	currDuty -= DUTY_STEP; // go below max
	printMsg("Ramping down. \r\n");
	while (currDuty >= ZERO_DUTY) {
		pwm_setDutyCycle(currDuty);
		printMsg("Current duty: %f \r\n", currDuty);
		delay_ms(250);
		currDuty -= DUTY_STEP;
	}

	delay_ms(1000); // wait to run
}
