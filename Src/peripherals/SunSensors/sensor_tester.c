#include <SunSensors/sun_sensors.h>
#include <globals.h>

void testFunction_INA_TMP_Outputs()
{
    float bus_voltage = sun_sensors_readBusVoltage(PANEL0);
    float shunt_voltage = sun_sensors_readShuntVoltage(PANEL0);
    float power = sun_sensors_readPower(PANEL0);
    float current = sun_sensors_readCurrent(PANEL0);
    float temp = sun_sensors_readTemp(PANEL0, TEMP0);
    float voltage = sun_sensors_readVoltage(PANEL0, DIODE0);
    
    
    while (1) {
        voltage = sun_sensors_readVoltage(PANEL0, DIODE1);
        bus_voltage = sun_sensors_readBusVoltage(PANEL0);
        shunt_voltage = sun_sensors_readShuntVoltage(PANEL0);
        power = sun_sensors_readPower(PANEL0);
        current = sun_sensors_readCurrent(PANEL0);
        temp = sun_sensors_readTemp(PANEL0, TEMP0);
    }


}

//10kOhms
