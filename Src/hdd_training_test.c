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
	led_d1(true);
	pwm_initTimer(10000); //This is in microseconds
	pwm_setDutyCycle(20); //20% of the power
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

	while (1) {
		 printMsg("This should start spamming. \r\n Working Setup! \r\n");
		 uint16_t adcVal = adc_singleConversion();
		 printMsg("ADC value: %d", adcVal);
		 printMsg(". MilliVolt value: ");
		 adc_printMilliVolt(adc_adcToVolt2(adcVal));
		 printMsg("\r\n");

	}
	delay_ms(10000);
	PWM_TIMER_OFF();
}

