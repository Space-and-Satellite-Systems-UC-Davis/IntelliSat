/**
 * power_distribution.c (Functions for managing power to peripherals)
 * 
 * - 6th May 2025
 *      Author : James W. Knepper
 *      Log    : Power distribution functions written
 */
#include "power_distribution.h"
#include <GPIO/gpio.h>

/**
 * Initialize I2C 
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