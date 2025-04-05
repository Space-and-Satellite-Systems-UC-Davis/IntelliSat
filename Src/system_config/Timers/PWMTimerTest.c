#include "timers.h"
#include <globals.h>
#include <print_scan.h>

 void testerFunction_PWMGeneration() {
 	pwm_initTimer(PWM0, 20000);
        pwm_initTimer(PWM1, 20000);

 	while (1) {
                pwm_timerOn(PWM0);
                pwm_setDutyCycle(PWM0, 20);
                delay_ms(10000);
                pwm_timerOff(PWM0);
                pwm_timerOn(PWM1);
                pwm_setDutyCycle(PWM1, 20);
                delay_ms(10000);
                pwm_timerOff(PWM1);
 	}
 }
