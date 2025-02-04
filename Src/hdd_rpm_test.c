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

	while (1) {
		 delay_ms(10);
		 printMsg("This should start spamming. \r\n Working Setup! \r\n");

	}
	delay_ms(10000);
	PWM_TIMER_OFF();
	adc_singleConversion();
}

