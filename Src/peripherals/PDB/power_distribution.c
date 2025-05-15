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
 * Turn on burn wire
 */
void pdb_pyro_on() {
    gpio_high(PDB_PWR_PYRO_PORT, PDB_PWR_PYRO_PIN);
}

/**
 * Turn off burn wire
 */
void pdb_pyro_off() {
    gpio_low(PDB_PWR_PYRO_PORT, PDB_PWR_PYRO_PIN);
}

/**
 * Reads the shunt voltage from the burn wire's INA226
 * @returns Voltage in mV
 */
float pdb_pyro_getShuntVoltage() {
    return INA226_getShuntVoltage(PDB_PYRO_GPIO, PDB_PYRO_SCL_PIN, PDB_PYRO_SDA_PIN, PDB_PYRO_ADDRESS);
}

/**
 * Reads the bus voltage from the burn wire's INA226
 * @returns Voltage in Volts
 */
float pdb_pyro_getBusVoltage() {
    return INA226_getBusVoltage(PDB_PYRO_GPIO, PDB_PYRO_SCL_PIN, PDB_PYRO_SDA_PIN, PDB_PYRO_ADDRESS);
}

/**
 * Reads the power from the burn wire's INA226
 * @returns Wattage as a 16 bit integer
 */
float pdb_pyro_getPower() {
    return INA226_getPower(PDB_PYRO_GPIO, PDB_PYRO_SCL_PIN, PDB_PYRO_SDA_PIN, PDB_PYRO_ADDRESS);
}

/**
 * Reads the current from the burn wire's INA226
 * @returns Amps as a 16 bit integer
 */
float pdb_pyro_getCurrent() {
    return INA226_getCurrent(PDB_PYRO_GPIO, PDB_PYRO_SCL_PIN, PDB_PYRO_SDA_PIN, PDB_PYRO_ADDRESS);
}

/**
 * Resets the mode of the burn wire's INA226
 * @param mode The mode of the sensor (continuous, powered down)
 * 
 * @returns None
 */
void pdb_pyro_setMode(int mode) {
    INA226_setMode(pdb_pyro, mode);
}

/**
 * Turn on magnetorquer
 */
void pdb_mgt_on() {
    gpio_high(PDB_PWR_MGT_PORT, PDB_PWR_MGT_PIN);
}

/**
 * Turn off magnetorquer
 */
void pdb_mgt_off() {
    gpio_low(PDB_PWR_MGT_PORT, PDB_PWR_MGT_PIN);
}

/**
 * Reads the shunt voltage from the magnetorquer's INA226
 * @returns Voltage in mV
 */
float pdb_mgt_getShuntVoltage() {
    return INA226_getShuntVoltage(PDB_MGT_GPIO, PDB_MGT_SCL_PIN, PDB_MGT_SDA_PIN, PDB_MGT_ADDRESS);
}

/**
 * Reads the bus voltage from the magnetorquer's INA226
 * @returns Voltage in Volts
 */
float pdb_mgt_getBusVoltage() {
    return INA226_getBusVoltage(PDB_MGT_GPIO, PDB_MGT_SCL_PIN, PDB_MGT_SDA_PIN, PDB_MGT_ADDRESS);
}

/**
 * Reads the power from the magnetorquer's INA226
 * @returns Wattage as a 16 bit integer
 */
float pdb_mgt_getPower() {
    return INA226_getPower(PDB_MGT_GPIO, PDB_MGT_SCL_PIN, PDB_MGT_SDA_PIN, PDB_MGT_ADDRESS);
}

/**
 * Reads the current from the magnetorquer's INA226
 * @returns Amps as a 16 bit integer
 */
float pdb_mgt_getCurrent() {
    return INA226_getCurrent(PDB_MGT_GPIO, PDB_MGT_SCL_PIN, PDB_MGT_SDA_PIN, PDB_MGT_ADDRESS);
}

/**
 * Resets the mode of the magnetorquer's INA226
 * @param mode The mode of the sensor (continuous, powered down)
 * 
 * @returns None
 */
void pdb_mgt_setMode(int mode) {
    INA226_setMode(pdb_mgt, mode);
}

/**
 * Turn on HDDs
 */
void pdb_hdd_on() {
    gpio_high(PDB_PWR_HDD_PORT, PDB_PWR_HDD_PIN);
}

/**
 * Turn off HDDs
 */
void pdb_hdd_off() {
    gpio_low(PDB_PWR_HDD_PORT, PDB_PWR_HDD_PIN);
}

/**
 * Reads the shunt voltage from the HDD's INA226
 * @returns Voltage in mV
 */
float pdb_hdd_getShuntVoltage() {
    return INA226_getShuntVoltage(PDB_HDD_GPIO, PDB_HDD_SCL_PIN, PDB_HDD_SDA_PIN, PDB_HDD_ADDRESS);
}

/**
 * Reads the bus voltage from the HDD's INA226
 * @returns Voltage in Volts
 */
float pdb_hdd_getBusVoltage() {
    return INA226_getBusVoltage(PDB_HDD_GPIO, PDB_HDD_SCL_PIN, PDB_HDD_SDA_PIN, PDB_HDD_ADDRESS);
}

/**
 * Reads the power from the HDD's INA226
 * @returns Wattage as a 16 bit integer
 */
float pdb_hdd_getPower() {
    return INA226_getPower(PDB_HDD_GPIO, PDB_HDD_SCL_PIN, PDB_HDD_SDA_PIN, PDB_HDD_ADDRESS);
}

/**
 * Reads the current from the HDD's INA226
 * @returns Amps as a 16 bit integer
 */
float pdb_hdd_getCurrent() {
    return INA226_getCurrent(PDB_HDD_GPIO, PDB_HDD_SCL_PIN, PDB_HDD_SDA_PIN, PDB_HDD_ADDRESS);
}

/**
 * Resets the mode of the HDD's INA226
 * @param mode The mode of the sensor (continuous, powered down)
 * 
 * @returns None
 */
void pdb_hdd_setMode(int mode) {
    INA226_setMode(pdb_hdd, mode);
}