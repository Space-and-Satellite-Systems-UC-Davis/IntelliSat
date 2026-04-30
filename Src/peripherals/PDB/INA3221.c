/*
 * INA3221.c
 *
 *  Created on: Apr 2, 2026
 */

#include "INA3221.h"

static int current_lsb; //in microamps
static int16_t config;

int ceiling2(float num){
    return num - (int)num > 0 ? (int)num + 1: (int)num;
}

void ina3221_init(int averages, int bus_time, int shunt_time, int rshunt, int max_current){
    softi2c_init(INA3221_GPIO, INA3221_SCL_PIN, INA3221_GPIO, INA3221_SDA_PIN);

    ina3221_config(averages, bus_time, shunt_time, rshunt, max_current);
}

void ina3221_config(int averages, int bus_time, int shunt_time, int rshunt, int max_current){
    config = averages << 9 | bus_time << 6 | shunt_time << 3 | MODE_CONTINUOUS;
    current_lsb = ceiling2(max_current / 32768.0 * MICRO); //2^15

//    softi2c_probe()
//    bool test = softi2c_probe(INA3221_GPIO, INA3221_SCL_PIN, INA3221_GPIO, INA3221_SDA_PIN, SENSOR_ADDRESS);


    //battery monitor
    softi2c_writeReg16(INA3221_GPIO, INA3221_SCL_PIN, INA3221_GPIO, INA3221_SDA_PIN, SENSOR_ADDRESS, CONFIG_REGISTER, 0x7127);

    softi2c_readReg16(INA3221_GPIO, INA3221_SCL_PIN, INA3221_GPIO, INA3221_SDA_PIN, SENSOR_ADDRESS, CONFIG_REGISTER);
}
