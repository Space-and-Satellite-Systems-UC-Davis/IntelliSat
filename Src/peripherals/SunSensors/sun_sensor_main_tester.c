#include <SunSensors/sun_sensors.h>
#include <globals.h>
#include <print_scan.h>
#include <Buttons/buttons.h>



void testFunction_Sun_Sensor_Main(){
    float n1[3] = {1.0, 1.0, 1.0};
    float n2[3] = {1.0, 1.0, 1.0};
    float n3[3] = {1.0, 1.0, 1.0};

    int readings[6] = {0,0,0,0,0,0};

    float vs[3] = {0.0,0.0,0.0};
	while (1) {
		if (get_buttonStatus_SW1()) {
            //for testing purposes: this will get readings from each diode on 3 panels
            //prints for panel 0 (side face)
            printMsg("Reading Voltage\r\n");
			float voltage00 = sun_sensors_readVoltage(PANEL0, DIODE0);
            printMsg("Voltage 1: %f\r\n", voltage00);
            float voltage01 = sun_sensors_readVoltage(PANEL0, DIODE1);
            printMsg("Voltage 2: %f\r\n", voltage01);

            //prints voltage readings for panel 1 (side face)
            printMsg("Reading Voltage\r\n");
			float voltage10 = sun_sensors_readVoltage(PANEL1, DIODE0);
            printMsg("Voltage 1: %f\r\n", voltage10);
            float voltage11 = sun_sensors_readVoltage(PANEL1, DIODE1);
            printMsg("Voltage 2: %f\r\n", voltage11);
            
            //prints voltage readings for panel 4 (top face)
            printMsg("Reading Voltage\r\n");
			float voltage40 = sun_sensors_readVoltage(PANEL4, DIODE0);
            printMsg("Voltage 1: %f\r\n", voltage40);
            float voltage41 = sun_sensors_readVoltage(PANEL4, DIODE1);
            printMsg("Voltage 2: %f\r\n", voltage41);
		}
	}
    

}