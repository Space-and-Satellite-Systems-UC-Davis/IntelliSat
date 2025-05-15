#include "timers.h"
#include <globals.h>
#include <print_scan.h>

 void testerFunction_PWMGeneration() {
 	while (1) {

 		for(int j = 5000; j<40000; j+=1000 ){
 		                        pwm_initTimer(PWM0, j);
 		                        pwm_initTimer(PWM1, j);
 		                        pwm_timerOn(PWM0);
 		                        pwm_setDutyCycle(PWM0, 30);
 		                        delay_ms(500);
 		                        pwm_timerOff(PWM0);
 		                        pwm_timerOn(PWM1);
 		                        pwm_setDutyCycle(PWM1, 30);
 		                        delay_ms(500);
 		                        pwm_timerOff(PWM1);
 		                        delay_ms(10);
 		                }
                pwm_initTimer(PWM0, 20000);
                pwm_initTimer(PWM1, 20000);
                for(int i = 0; i<=100; i++){
                        pwm_timerOn(PWM0);
                        pwm_setDutyCycle(PWM0, i);
                        delay_ms(1000);
                        pwm_timerOff(PWM0);
                        pwm_timerOn(PWM1);
                        pwm_setDutyCycle(PWM1, i);
                        delay_ms(1000);
                        pwm_timerOff(PWM1);
                        delay_ms(10);
                }  

                  
 	}
 }
