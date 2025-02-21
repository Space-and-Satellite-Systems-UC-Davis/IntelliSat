#include "INA226.h"
#include <globals.h>
#include <Buttons/buttons.h>

void testFunction_INA_Outputs()
{
    sensor_init(AVERAGE_16, CONVERT_204, CONVERT_332, MODE_CONTINUOUS, 68,30);
    int id = get_id();
    float bus_voltage = get_bus_voltage();
    float shunt_voltage = get_shunt_voltage();
    float power = get_power();
    float current = get_current();
    int config = get_config();
    
    while (1) {
    	if(get_buttonStatus_1()){
            id = get_id();
            bus_voltage = get_bus_voltage();
            shunt_voltage = get_shunt_voltage();
            power = get_power();
            current = get_current();
        }
    }


}

//10kOhms
