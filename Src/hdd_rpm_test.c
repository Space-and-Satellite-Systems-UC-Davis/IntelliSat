#include <Timers/timers.h>
#include <ADC/adc.h>
#include <LED/led.h>
// any necessary #include

void testFunction_HDD_RPM() {
	// any initializations
	led_d1(true);
	pwm_initTimer(10000); //This is in microseconds
	pwm_setDutyCycle(16); //20% of the power
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
		 delay_ms(10);
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

