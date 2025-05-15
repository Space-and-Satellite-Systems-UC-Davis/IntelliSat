/**
 * power_distribution.c (Functions for managing power to peripherals)
 * 
 * - 6th May 2025
 *      Author : James W. Knepper
 *      Log    : Power distribution functions written
 */
#include "power_distribution.h"
#include <INA226/INA226.h>
#include <GPIO/gpio.h>

INA226_I2C_Interface pdb_pyro_ina226, pdb_mgt_ina226, pdb_hdd_ina226, batmon_ina226;

INA226_I2C_Interface * pdb_pyro = &pdb_pyro_ina226;
INA226_I2C_Interface * pdb_mgt = &pdb_mgt_ina226;
INA226_I2C_Interface * pdb_hdd = &pdb_hdd_ina226;
INA226_I2C_Interface * batmon = &batmon_ina226;

/**
 * Initialize I2C and power control pins
 */
void pdb_init() {
    // PDB_PWR_^_PORT->MODER &= ~(GPIO_MODER_Analog << (PDB_PWR_^_PIN*GPIO_MODER_BITS_SPAN));
    // PDB_PWR_^_PORT->MODER |= (GPIO_MODER_Output & GPIO_MODER_Analog) << (PDB_PWR_^_PIN*GPIO_MODER_BITS_SPAN);
    PDB_PWR_PYRO_PORT->MODER &= ~(GPIO_MODER_Analog << (PDB_PWR_PYRO_PIN*GPIO_MODER_BITS_SPAN));
    PDB_PWR_PYRO_PORT->MODER |= (GPIO_MODER_Output & GPIO_MODER_Analog) << (PDB_PWR_PYRO_PIN*GPIO_MODER_BITS_SPAN);
    PDB_PWR_MGT_PORT->MODER &= ~(GPIO_MODER_Analog << (PDB_PWR_MGT_PIN*GPIO_MODER_BITS_SPAN));
    PDB_PWR_MGT_PORT->MODER |= (GPIO_MODER_Output & GPIO_MODER_Analog) << (PDB_PWR_MGT_PIN*GPIO_MODER_BITS_SPAN);
    PDB_PWR_HDD_PORT->MODER &= ~(GPIO_MODER_Analog << (PDB_PWR_HDD_PIN*GPIO_MODER_BITS_SPAN));
    PDB_PWR_HDD_PORT->MODER |= (GPIO_MODER_Output & GPIO_MODER_Analog) << (PDB_PWR_HDD_PIN*GPIO_MODER_BITS_SPAN);
    
    // initialize pyro
    pdb_pyro->address = PDB_PYRO_ADDRESS;
    pdb_pyro->SCL_GPIO = PDB_PYRO_GPIO;
    pdb_pyro->SCL_pin = PDB_PYRO_SCL_PIN;
    pdb_pyro->SDA_GPIO = PDB_PYRO_GPIO;
    pdb_pyro->SDA_pin = PDB_PYRO_SDA_PIN;
    INA226_init(pdb_pyro, AVERAGE_1, CONVERT_1100, CONVERT_1100, 68, 30);

    // initialize mgt
    pdb_mgt->address = PDB_MGT_ADDRESS;
    pdb_mgt->SCL_GPIO = PDB_MGT_GPIO;
    pdb_mgt->SCL_pin = PDB_MGT_SCL_PIN;
    pdb_mgt->SDA_GPIO = PDB_MGT_GPIO;
    pdb_mgt->SDA_pin = PDB_MGT_SDA_PIN;
    INA226_init(pdb_mgt, AVERAGE_1, CONVERT_1100, CONVERT_1100, 68, 30);

    // initialize hdd
    pdb_hdd->address = PDB_MGT_ADDRESS;
    pdb_hdd->SCL_GPIO = PDB_HDD_GPIO;
    pdb_hdd->SCL_pin = PDB_HDD_SCL_PIN;
    pdb_hdd->SDA_GPIO = PDB_HDD_GPIO;
    pdb_hdd->SDA_pin = PDB_HDD_SDA_PIN;
    INA226_init(pdb_hdd, AVERAGE_1, CONVERT_1100, CONVERT_1100, 68, 30);

    // initialize batmon
    batmon->address = BATMON_ADDRESS;
    batmon->SCL_GPIO = BATMON_GPIO;
    batmon->SCL_pin = BATMON_SCL;
    batmon->SDA_GPIO = BATMON_GPIO;
    batmon->SDA_pin = BATMON_SDA;
    INA226_init(batmon, AVERAGE_1, CONVERT_1100, CONVERT_1100, 68, 30);
}

/**
 * Turn on HDDs
 */
void pdb_on(PDB_PERIPHERAL peripheral) {
    switch (peripheral) {
        case PDB_PERIPHERAL_PYRO: gpio_high(PDB_PWR_PYRO_PORT, PDB_PWR_PYRO_PIN);
        case PDB_PERIPHERAL_MGT: gpio_high(PDB_PWR_MGT_PORT, PDB_PWR_MGT_PIN);
        case PDB_PERIPHERAL_HDD: gpio_high(PDB_PWR_HDD_PORT, PDB_PWR_HDD_PIN);
    }
}

/**
 * Turn off HDDs
 */
void pdb_off(PDB_PERIPHERAL peripheral) {
    switch (peripheral) {
        case PDB_PERIPHERAL_PYRO: gpio_low(PDB_PWR_PYRO_PORT, PDB_PWR_PYRO_PIN);
        case PDB_PERIPHERAL_MGT: gpio_low(PDB_PWR_MGT_PORT, PDB_PWR_MGT_PIN);
        case PDB_PERIPHERAL_HDD: gpio_low(PDB_PWR_HDD_PORT, PDB_PWR_HDD_PIN);
    }
}

/**
 * Reads the shunt voltage from a peripheral's INA226
 * @returns Voltage in mV
 */
float pdb_getShuntVoltage(PDB_PERIPHERAL peripheral) {
    PDB_PERIPHERAL * interface;
    switch (peripheral) {
        case PDB_PERIPHERAL_PYRO: interface = pdb_pyro;
        case PDB_PERIPHERAL_MGT: interface = pdb_mgt;
        case PDB_PERIPHERAL_HDD: interface = pdb_hdd;
        case PDB_PERIPHERAL_BATMON: interface = batmon;
        default: return -1;
    }
    return INA226_getShuntVoltage(interface);
}

/**
 * Reads the bus voltage from the HDD's INA226
 * @returns Voltage in Volts
 */
float pdb_getBusVoltage(PDB_PERIPHERAL peripheral) {
    PDB_PERIPHERAL * interface;
    switch (peripheral) {
        case PDB_PERIPHERAL_PYRO: interface = pdb_pyro;
        case PDB_PERIPHERAL_MGT: interface = pdb_mgt;
        case PDB_PERIPHERAL_HDD: interface = pdb_hdd;
        case PDB_PERIPHERAL_BATMON: interface = batmon;
        default: return -1;
    }
    return INA226_getBusVoltage(interface);
}

/**
 * Reads the power from the HDD's INA226
 * @returns Wattage as a 16 bit integer
 */
float pdb_getPower(PDB_PERIPHERAL peripheral) {
    PDB_PERIPHERAL * interface;
    switch (peripheral) {
        case PDB_PERIPHERAL_PYRO: interface = pdb_pyro;
        case PDB_PERIPHERAL_MGT: interface = pdb_mgt;
        case PDB_PERIPHERAL_HDD: interface = pdb_hdd;
        case PDB_PERIPHERAL_BATMON: interface = batmon;
        default: return -1;
    }
    return INA226_getPower(interface);
}

/**
 * Reads the current from the HDD's INA226
 * @returns Amps as a 16 bit integer
 */
float pdb_getCurrent(PDB_PERIPHERAL peripheral) {
    PDB_PERIPHERAL * interface;
    switch (peripheral) {
        case PDB_PERIPHERAL_PYRO: interface = pdb_pyro;
        case PDB_PERIPHERAL_MGT: interface = pdb_mgt;
        case PDB_PERIPHERAL_HDD: interface = pdb_hdd;
        case PDB_PERIPHERAL_BATMON: interface = batmon;
        default: return -1;
    }
    return INA226_getCurrent(interface);
}

/**
 * Resets the mode of the HDD's INA226
 * @param mode The mode of the sensor (continuous, powered down)
 * 
 * @returns None
 */
void pdb_setMode(PDB_PERIPHERAL peripheral, int mode) {
    PDB_PERIPHERAL * interface;
    switch (peripheral) {
        case PDB_PERIPHERAL_PYRO: interface = pdb_pyro;
        case PDB_PERIPHERAL_MGT: interface = pdb_mgt;
        case PDB_PERIPHERAL_HDD: interface = pdb_hdd;
        case PDB_PERIPHERAL_BATMON: interface = batmon;
        default: return -1;
    }
    INA226_setMode(interface, mode);
}