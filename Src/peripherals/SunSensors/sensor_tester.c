#include <SunSensors/sun_sensors.h>
#include <globals.h>

void testFunction_INA_TMP_Outputs()
{
    float bus_voltage = read_bus_voltage(PANEL0);
    float shunt_voltage = read_shunt_voltage(PANEL0);
    float power = read_power(PANEL0);
    float current = read_current(PANEL0);
    float temp = read_temp(PANEL0, TEMP0);
    float voltage = sun_sensors_readVoltage(PANEL0, DIODE0);
    
    
    while (1) {
        voltage = sun_sensors_readVoltage(PANEL0, DIODE1);
        bus_voltage = read_bus_voltage(PANEL0);
        shunt_voltage = read_shunt_voltage(PANEL0);
        power = read_power(PANEL0);
        current = read_current(PANEL0);
        temp = read_temp(PANEL0, TEMP0);
    }


}

//10kOhms
