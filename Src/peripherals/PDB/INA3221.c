/*
 * INA3221.c
 *
 *  Created on: Apr 2, 2026
 */

#include "INA3221.h"
#include "I2C/i2c.h"
#include <stdint.h>

/*************************** Parameters *************************/
static int16_t config;

int ceil(float num){
    return num - (int)num > 0 ? (int)num + 1: (int)num;
}

void ina3221_init(int averages, int bus_time, int shunt_time) {
    softi2c_init(INA3221_SCL_PORT, INA3221_SCL_PIN, INA3221_SDA_PORT,
        INA3221_SDA_PIN);
    ina3221_config(averages, bus_time, shunt_time);
}

void ina3221_config(int averages, int bus_time, int shunt_time){
    // TODO: Check later. Must enable all channels. Technically identical
    config = 0x7 << 12 // Enable all channels
        | averages << 9 
        | bus_time << 6 
        | shunt_time << 3
        | SHUNT_BUS_CONTINUOUS << 0;
        // | SHUNT_SINGLE_SHOT;

    //Monitor 0
    softi2c_writeReg16(INA3221_SCL_PORT, INA3221_SCL_PIN, 
        INA3221_SDA_PORT, INA3221_SDA_PIN, 
        INA3221_ADDR_0, CONFIG_REG, config);

    //Monitor 1
    softi2c_writeReg16(INA3221_SCL_PORT, INA3221_SCL_PIN, 
        INA3221_SDA_PORT, INA3221_SDA_PIN, 
        INA3221_ADDR_1, CONFIG_REG, config);
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

float ina3221_getPower(uint8_t address, INA3221_REG channel,
    GPIO_TypeDef* gpio, int scl_pin, int sda_pin){
    int16_t output = softi2c_readReg16(gpio,scl_pin, gpio, sda_pin, address, channel);
    return output;
    // return output * current_lsb/(double)MICRO * 25; 
}

float ina3221_getCurrent(uint8_t address, INA3221_REG channel,
    GPIO_TypeDef* gpio, int scl_pin, int sda_pin){
    int16_t output = softi2c_readReg16(gpio,scl_pin, gpio, sda_pin, address, channel);
    return output;
    // return output * current_lsb / (double)MICRO;
}

void ina3221_setMode(uint8_t mode){
    int16_t changed_config = (config & ~0x7) | mode;

    //Monitor 0
    softi2c_writeReg16(INA3221_SCL_PORT, INA3221_SCL_PIN, 
        INA3221_SDA_PORT, INA3221_SDA_PIN, 
        INA3221_ADDR_0, CONFIG_REG, changed_config);

    //Monitor 1
    softi2c_writeReg16(INA3221_SCL_PORT, INA3221_SCL_PIN, 
        INA3221_SDA_PORT, INA3221_SDA_PIN, 
        INA3221_ADDR_1, CONFIG_REG, changed_config);
}

uint16_t ina3221_getDieID(uint8_t address, GPIO_TypeDef* scl_port, 
    int scl_pin, GPIO_TypeDef* sda_port, int sda_pin) {    
    // Set register pointer
    // softi2c_setRegister(scl_port, scl_pin, sda_port, sda_pin, address,
    //     DIE_ID_REG);

    uint16_t id = softi2c_readReg16(scl_port, scl_pin, sda_port, sda_pin, address, DIE_ID_REG);

    return id;
}
