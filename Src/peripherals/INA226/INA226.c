#include "INA226.h"

/*************************** Parameters *************************/
static int current_lsb; //in microamps
static int16_t config;

int ceiling(float num){
    return num - (int)num > 0 ? (int)num + 1: (int)num;
}

void INA226_init(INA226_I2C_Interface * interface, int averages, int bus_time, int shunt_time, int rshunt, int max_current){
    softi2c_init(interface->SCL_GPIO, interface->SCL_pin, interface->SDA_GPIO, interface->SDA_pin);
    INA226_config(interface, averages, bus_time, shunt_time, rshunt, max_current);
}

void INA226_config(INA226_I2C_Interface * interface, int averages, int bus_time, int shunt_time, int rshunt, int max_current){
    config = averages << 9 | bus_time << 6 | shunt_time << 3 | MODE_CONTINUOUS;
    current_lsb = ceiling(max_current / 32768.0 * MICRO); //2^15
    int16_t cal = ceiling((0.00512 / (current_lsb * 1.0 /MICRO)) /(rshunt * 1.0 /MILLI)); //round to nearest integer above

    softi2c_writeReg16(interface->SCL_GPIO, interface->SCL_pin, interface->SDA_GPIO, interface->SDA_pin, interface->address, CONFIG_REGISTER, config); // configuration register
    softi2c_writeReg16(interface->SCL_GPIO, interface->SCL_pin, interface->SDA_GPIO, interface->SDA_pin, interface->address, CAL_REGISTER, cal); //calibration register
}

//no need to trigger, only use continuous mode or power-down mode
/*void trigger(){
    bool powered = current_sensor.mode == MODE_POWERED_DOWN;
    set_mode(MODE_TRIGGERED, false);
    int ready = 0;
    while(!ready)
    {
        ready = softi2c_readReg16(current_sensor.SCL_GPIO, current_sensor.SCL_PIN, current_sensor.SDA_GPIO, current_sensor.SDA_PIN, SENSOR_ADDRESS, 6);
        ready = (ready >> 3) & 1;
    }
    if(powered)
        set_mode(MODE_POWERED_DOWN, false);
}*/

float INA226_getShuntVoltage(INA226_I2C_Interface * interface){
    int16_t output = softi2c_readReg16(interface->SCL_GPIO, interface->SCL_GPIO, interface->SDA_GPIO, interface->SDA_pin, interface->address, 0x01);
    return output * .0025;
}


float INA226_getBusVoltage(INA226_I2C_Interface * interface){
    int16_t output = softi2c_readReg16(interface->SCL_GPIO, interface->SCL_GPIO, interface->SDA_GPIO, interface->SDA_pin, interface->address, 0x02);
    return output * 1.25/MILLI; 
}


float INA226_getPower(INA226_I2C_Interface * interface){
    int16_t output = softi2c_readReg16(interface->SCL_GPIO, interface->SCL_GPIO, interface->SDA_GPIO, interface->SDA_pin, interface->address, 0x03);
    return output * current_lsb/(double)MICRO * 25; 
}


float INA226_getCurrent(INA226_I2C_Interface * interface){
    int16_t output = softi2c_readReg16(interface->SCL_GPIO, interface->SCL_GPIO, interface->SDA_GPIO, interface->SDA_pin, interface->address, 0x04);
    return output * current_lsb / (double)MICRO;
}


void INA226_setMode(INA226_I2C_Interface * interface, int mode){
    int16_t changed_config = (config & ~7) | mode;
    softi2c_writeReg16(interface->SCL_GPIO, interface->SCL_pin, interface->SDA_GPIO, interface->SDA_pin, interface->address, 0, changed_config);
}
