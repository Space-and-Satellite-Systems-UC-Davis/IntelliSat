/*
 * TestID: 9
 *  Date: 2025-08-23
 *  Author: Soren Keck
 *  Notes:
 *       This code contains a basic function for reading the value of the photodiode plugged into Sol-0
 *       when the button is pressed
 */

#include <SunSensors/sun_sensors.h>
#include <globals.h>
#include <print_scan.h>
#include <Buttons/buttons.h>


void testFunction_SS(){

    while (1) {
        if (get_buttonStatus_SW1()) {
            float d0 = sun_sensors_readVoltage(PANEL0, DIODE0);
            printMsg("Voltage 1: %f\r\n", d0);
            float d1 = sun_sensors_readVoltage(PANEL0, DIODE1);
            printMsg("Voltage 2: %f\r\n", d1);
        }
    }
}
