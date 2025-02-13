#include "INA226.h"
#include <globals.h>

void testFunction_INA_Outputs()
{
    sensor_init(AVERAGE_1, CONVERT_1100, CONVERT_1100, MODE_CONTINUOUS, 22,10);
    int id = get_id();
    float voltage = get_bus_voltage();
    select_sensor(1);
    id = get_id();
    voltage = get_bus_voltage();
    select_sensor(2);
    id = get_id();
    voltage = get_bus_voltage();
    select_sensor(3);
    id = get_id();
    voltage = get_bus_voltage();
    select_sensor(0);
    id = get_id();
    voltage = get_bus_voltage();

}

//10kOhms
