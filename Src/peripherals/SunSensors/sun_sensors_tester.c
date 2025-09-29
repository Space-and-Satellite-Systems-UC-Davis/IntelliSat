#include "virtual_intellisat.h"
#include <SunSensors/sun_sensors.h>
#include <globals.h>
#include <print_scan.h>
#include <Buttons/buttons.h>
#include "../system_config/Timers/timers.h"

void testFunction_Diode_Reads() {
    printMsg("Starting Diode reading\r\n");
    printMsg("Reading Voltage\r\n");
	
    int doPrint = 0;
    uint32_t lastPrintTimeMs = 0;
    uint32_t currTimeMs = 0;
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
