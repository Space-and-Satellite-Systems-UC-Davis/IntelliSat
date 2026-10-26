/*
 * INA3221.c
 *
 *  Created on: Apr 2, 2026
 */

#include "INA3221.h"

/*************************** Parameters *************************/
static int16_t config;

void ina3221_init(int averages, int bus_time, int shunt_time, uint8_t mode) {
    softi2c_init(INA3221_SCL_PORT, INA3221_SCL_PIN, INA3221_SDA_PORT,
        INA3221_SDA_PIN);
    ina3221_config(averages, bus_time, shunt_time, mode);
}

void ina3221_config(int averages, int bus_time, int shunt_time, uint8_t mode){
    config = 0x7 << 12 // Enable all channels
        | averages << 9 
        | bus_time << 6 
        | shunt_time << 3
        | mode << 0;

    //Monitor 0
    softi2c_writeReg16(INA3221_SCL_PORT, INA3221_SCL_PIN, 
        INA3221_SDA_PORT, INA3221_SDA_PIN, 
        INA3221_ADDR_0, CONFIG_REG, config);

    //Monitor 1
    softi2c_writeReg16(INA3221_SCL_PORT, INA3221_SCL_PIN, 
        INA3221_SDA_PORT, INA3221_SDA_PIN, 
        INA3221_ADDR_1, CONFIG_REG, config);
}

float ina3221_getBusVoltage(uint8_t address, INA3221_REG channel,
		GPIO_TypeDef* scl_port, int scl_pin, 
        GPIO_TypeDef* sda_port, int sda_pin) {

    // If single-shot, touch config to cause conversion
    // WARNING: Might be different behavior if powerdown allowed in enum?
    if ((config & INA_CFG_MODE_MSK) < SHUNT_CONTINUOUS) {
        softi2c_writeReg16(INA3221_SCL_PORT, INA3221_SCL_PIN, 
            INA3221_SDA_PORT, INA3221_SDA_PIN, 
            address, CONFIG_REG, config);

        int16_t cvrf = 0;
        uint64_t start = getSysTime();
        while ((cvrf == 0) && !is_time_out(start, DEFAULT_TIMEOUT_MS)) {
            int16_t reg = softi2c_readReg16(scl_port, scl_pin, sda_port,
                sda_pin, address, MASK_ENABLE_REG);
            cvrf = (reg & INA_CVRF_MSK);
        }
    }

    int16_t raw = softi2c_readReg16(scl_port, scl_pin, sda_port, sda_pin, address, channel);

    return ((float) raw) / 1000;
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
    uint16_t id = softi2c_readReg16(scl_port, scl_pin, sda_port, sda_pin, address, DIE_ID_REG);

    return id;
}
