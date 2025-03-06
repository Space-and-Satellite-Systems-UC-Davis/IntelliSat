#include <SunSensors/sun_sensors.h>
#include <globals.h>

void testFunction_INA_TMP_Outputs()
{
    sun_sensor_init();
    float bus_voltage = read_bus_voltage(PANEL0);
    float shunt_voltage = read_shunt_voltage(PANEL0);
    float power = read_power(PANEL0);
    float current = read_current(PANEL0);
    float temp = read_temp(PANEL0, TEMP0);
    
    
    while (1) {
        bus_voltage = read_bus_voltage(PANEL0);
        shunt_voltage = read_shunt_voltage(PANEL0);
        power = read_power(PANEL0);
        current = read_current(PANEL0);
        temp = read_temp(PANEL0, TEMP0);
    }


}

//10kOhms
