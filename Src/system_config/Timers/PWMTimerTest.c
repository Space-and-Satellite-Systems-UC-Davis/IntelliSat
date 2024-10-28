#include "timers.h"
#include <globals.h>
#include <print_scan.h>

 void testerFunction_PWMGeneration() {
 	pwm_initTimer(20000);

 	while (1) {
         PWM_TIMER_ON();
         pwm_setDutyCycle(1);
         delay_ms(10000);
         PWM_TIMER_OFF();
         delay_ms(10000);
 	}
 }
