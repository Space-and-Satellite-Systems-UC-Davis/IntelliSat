
#include <SunSensors/sun_sensors.h>
#include <globals.h>
#include <print_scan.h>
#include <Buttons/buttons.h>

void testFunction_Diode_Reads() {
    printMsg("Starting Diode reading\r\n");
    sun_sensor_init();
	
	while (1) {
		if (get_buttonStatus_SW1()) {
            printMsg("Reading Voltage\r\n");
			uint16_t voltage = sun_sensors_readVoltage(PANEL0, DIODE0);
            printMsg("Voltage: %d\r\n", voltage);

		}
	}
}
