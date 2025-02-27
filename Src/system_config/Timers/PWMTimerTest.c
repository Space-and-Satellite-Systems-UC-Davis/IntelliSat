#include "timers.h"
#include <globals.h>
#include <print_scan.h>
#include <LED/led.h>
void testerFunction_PWMGeneration() {

        pwm_initTimer(20000);

        while (1) {
                for (int i  = 0; i < 100; i++){
                        PWM_TIMER_ON();
                        pwm_setDutyCycle(i);
                        delay_ms(500);
                }
}    
 }
