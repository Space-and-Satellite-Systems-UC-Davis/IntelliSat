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
 * Turn on magnetorquer
 */
void pdb_mgt_on();

/**
 * Turn off magnetorquer
 */
void pdb_mgt_off();

/**
 * Turn on HDDs
 */
void pdb_hdd_on();

/**
 * Turn off HDDs
 */
void pdb_hdd_off();