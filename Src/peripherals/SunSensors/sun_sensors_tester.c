
#include <SunSensors/sun_sensors.h>
#include <globals.h>
#include <print_scan.h>
#include <Buttons/buttons.h>

void testFunction_Diode_Reads() {
    printMsg("Starting Diode reading\n");
	
	while (1) {
		if (get_buttonStatus_SW1()) {
            printMsg("Reading Voltage\n");
			float voltage = sun_sensors_readVoltage(PANEL0, DIODE0);
            printMsg("Voltage 1: %f\n", voltage);
            voltage = sun_sensors_readVoltage(PANEL0, DIODE1);
            printMsg("Voltage 2: %f\n", voltage);
		}
	}
}
