#include "virtual_intellisat.h"
#include <SunSensors/sun_sensors.h>
#include <globals.h>
#include <print_scan.h>
#include <Buttons/buttons.h>
#include "../system_config/Timers/timers.h"

#include <../hdd/hdd_drive.h>
#include <../hdd/hdd_init.h>

void testFunction_Diode_Reads() {
    printMsg("Starting Diode reading\r\n");
    printMsg("Reading Voltage\r\n");

    hdd_init(PWM0);
    hdd_arm(PWM0);
    pwm_setDutyCycle(PWM0, 100);

    printMsg("Testing mid duty \r\n");
    pwm_setDutyCycle(PWM0, 50);
    delay_ms(5000);

    int numLoops = 100;
    while (numLoops != 0) {
    	pwm_setDutyCycle(PWM0, 100);
    	delay_ms(3000);
    	pwm_setDutyCycle(PWM0, 50);
    	delay_ms(1000);
    	printMsg("Loops remaining: %d\r\n", numLoops);
    	--numLoops;
    }

    printMsg("Testing max duty \r\n");
    pwm_setDutyCycle(PWM0, 75);
    delay_ms(10000);
    printMsg("Finished max duty setting\r\n");
	
    int doPrint = 0;
    uint32_t lastPrintTimeMs = 0;
    uint64_t currTimeMs = 0;
    vi_get_curr_millis(&currTimeMs);
	while (1) {
		float voltage = sun_sensors_readVoltage(PANEL0, DIODE0);
		printMsg("V1: %f ", voltage);
		voltage = sun_sensors_readVoltage(PANEL0, DIODE1);
		printMsg("V2: %f\r\n", voltage);
		delay_ms(1000);
		// if (get_buttonStatus_SW1()) {}
	}
}
