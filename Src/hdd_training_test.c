/*
 * hdd_training_test.c
 *
 *  Created on: Sep 25, 2024
 *      Author: Chandrark Muddana
 */
#include <Timers/timers.h>
#include <ADC/adc.h>
#include <LED/led.h>

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
	pwm_initTimer(10000); //This is in microseconds
	pwm_setDutyCycle(16); //20% of the power
	PWM_TIMER_ON();

	while (1) {
		 delay_ms(10);
		 printMsg("This should start spamming. \r\n Working Setup! \r\n");
		 led_d1(true);
	}
	delay_ms(10000);
	PWM_TIMER_OFF();
	adc_singleConversion();
}

