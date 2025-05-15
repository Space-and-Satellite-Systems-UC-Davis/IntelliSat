/**
 * power_distribution.h (Functions for managing power to peripherals)
 * 
 * - 6th May 2025
 *      Author : James W. Knepper
 *      Log    : Power distribution functions written
 */
#pragma once

#include "stm32l476xx.h"

#define PDB_PWR_PYRO_PORT GPIOC
#define PDB_PWR_PYRO_PIN 9
#define PDB_PWR_MGT_PORT GPIOA
#define PDB_PWR_MGT_PIN 8
#define PDB_PWR_HDD_PORT GPIOA
#define PDB_PWR_HDD_PIN 9
//#define PDB_PWR_^_PORT GPIOx
//#define PDB_PWR_^_PIN n

// Main INA226 bus access data
#define PDB_GPIO GPIOC
#define PDB_SCL_PIN 7
#define PDB_SDA_PIN 8

// Pyro specific access data
#define PDB_PYRO_ADDRESS 0x45 // TODO
#define PDB_PYRO_GPIO PDB_GPIO
#define PDB_PYRO_SCL_PIN PDB_SCL_PIN
#define PDB_PYRO_SDA_PIN PDB_SDA_PIN

// MGT specific access data
#define PDB_MGT_ADDRESS 0x45 // TODO
#define PDB_MGT_GPIO PDB_GPIO
#define PDB_MGT_SCL_PIN PDB_SCL_PIN
#define PDB_MGT_SDA_PIN PDB_SDA_PIN

// HDD specific access data
#define PDB_HDD_ADDRESS 0x45 // TODO
#define PDB_HDD_GPIO PDB_GPIO
#define PDB_HDD_SCL_PIN PDB_SCL_PIN
#define PDB_HDD_SDA_PIN PDB_SDA_PIN

// Battery monitor pins
#define BATMON_ADDRESS 0x45 // TODO
#define BATMON_GPIO GPIOH
#define BATMON_SDA 0
#define BATMON_SCL 1

/**
 * Initialize power pins for output
 */
void pdb_init();

/**
 * Turn on burn wire
 */
void pdb_pyro_on();

/**
 * Turn off burn wire
 */
void pdb_pyro_off();

/**
 * Reads the shunt voltage from the burn wire's INA226
 * @returns Voltage in mV
 */
float pdb_pyro_getShuntVoltage();

/**
 * Reads the bus voltage from the burn wire's INA226
 * @returns Voltage in Volts
 */
float pdb_pyro_getBusVoltage();

/**
 * Reads the power from the burn wire's INA226
 * @returns Wattage as a 16 bit integer
 */
float pdb_pyro_getPower();

/**
 * Reads the current from the burn wire's INA226
 * @returns Amps as a 16 bit integer
 */
float pdb_pyro_getCurrent();

/**
 * Resets the mode of the burn wire's INA226
 * @param mode The mode of the sensor (continuous, powered down)
 * 
 * @returns None
 */
void pdb_pyro_setMode(int mode);

/**
 * Turn on magnetorquer
 */
void pdb_mgt_on();

/**
 * Turn off magnetorquer
 */
void pdb_mgt_off();

/**
 * Reads the shunt voltage from the magentorquer's INA226
 * @returns Voltage in mV
 */
float pdb_mgt_getShuntVoltage();

/**
 * Reads the bus voltage from the magentorquer's INA226
 * @returns Voltage in Volts
 */
float pdb_mgt_getBusVoltage();

/**
 * Reads the power from the magnetorquer's INA226
 * @returns Wattage as a 16 bit integer
 */
float pdb_mgt_getPower();

/**
 * Reads the current from the magnetorquer's INA226
 * @returns Amps as a 16 bit integer
 */
float pdb_mgt_getCurrent();

/**
 * Resets the mode of the magnetorquer's INA226
 * @param mode The mode of the sensor (continuous, powered down)
 * 
 * @returns None
 */
void pdb_mgt_setMode(int mode);

/**
 * Turn on HDDs
 */
void pdb_hdd_on();

/**
 * Turn off HDDs
 */
void pdb_hdd_off();

/**
 * Reads the shunt voltage from the HDD's INA226
 * @returns Voltage in mV
 */
float pdb_hdd_getShuntVoltage();

/**
 * Reads the bus voltage from the HDD's INA226
 * @returns Voltage in Volts
 */
float pdb_hdd_getBusVoltage();

/**
 * Reads the power from the HDD's INA226
 * @returns Wattage as a 16 bit integer
 */
float pdb_hdd_getPower();

/**
 * Reads the current from the HDD's INA226
 * @returns Amps as a 16 bit integer
 */
float pdb_hdd_getCurrent();

/**
 * Resets the mode of the HDD's INA226
 * @param mode The mode of the sensor (continuous, powered down)
 * 
 * @returns None
 */
void pdb_hdd_setMode(int mode);