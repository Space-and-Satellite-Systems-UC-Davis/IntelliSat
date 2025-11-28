#include "print_scan.h"
#include <SunSensors/sun_sensors.h>
#include <globals.h>

void testFunction_INA_TMP_Outputs()
{      
    while (1) {
        float voltage = sun_sensors_readVoltage(PANEL0, DIODE1);
        float bus_voltage = sun_sensors_readBusVoltage(PANEL0);
        float shunt_voltage = sun_sensors_readShuntVoltage(PANEL0);
        float power = sun_sensors_readPower(PANEL0);
        float current = sun_sensors_readCurrent(PANEL0);
        float temperature = sun_sensors_readTemp(PANEL0, TEMP0);
        printMsg("Voltage: %dV",voltage);
        printMsg("Bus Voltage: %dV",bus_voltage);
        printMsg("Shunt Voltage: %dmV",shunt_voltage);
        printMsg("Power: %dW",power);
        printMsg("Current: %dA",current);
        printMsg("Temp: %dC",temperature);
    }


}

//10kOhms
