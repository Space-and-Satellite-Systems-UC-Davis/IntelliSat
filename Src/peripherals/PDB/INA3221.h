/*
 * INA3221.h
 *
 *  Created on: Apr 2, 2026
 */

#ifndef INA3221_H_
#define INA3221_H_

#include <I2C/i2c.h>

//Battery INA address
#define INA3221_ADDR_0 0x40

// The other one
#define INA3221_ADDR_1 0x41

typedef enum INA3221_REG_ENUM {
    CONFIG_REG = 0x0,
    CH1_SHUNT_REG = 0x01,
    // CH1_BUS_REG = 0x02,
    CH2_SHUNT_REG = 0x03,
    // CH2_BUS_REG = 0x04,
    CH3_SHUNT_REG = 0x05,
    // CH3_BUS_REG = 0x06,
    FAB_ID_REG = 0xFE,
    DIE_ID_REG = 0xFF
}INA3221_REG;

typedef enum INA3221_MODE_ENUM {
    SHUNT_SINGLE_SHOT = 0b001,
    SHUNT_CONTINUOUS = 0b101
}INA3221_REG;


//Macros for the INA3221 pins
#define INA3221_SDA_PORT GPIOC
#define INA3221_SCL_PORT GPIOC
#define INA3221_SDA_PIN 8
#define INA3221_SCL_PIN 7

//number conversions
#define MICRO 1000000
#define MILLI 1000


/**
 * Initializes PDB voltage monitor with I2C and calls config function
 *
 * @param averages Number of samples to collect and average
 * @param bus_time The conversion time for bus voltage measurements
 * @param shunt_time The conversion time for shunt voltage measurements
 * @param rshunt The value of the external shunt used in milliohms
 * @param max_current The value of the maximum current
 *
 * @returns None
 */
void ina3221_init(int averages, int bus_time, int shunt_time, int rshunt, int max_current);

/**
 * Initial configuration of the sun sensor's config register and calibration register
 *
 * @param averages Number of samples to collect and average
 * @param bus_time The conversion time for bus voltage measurements
 * @param shunt_time The conversion time for shunt voltage measurements
 * @param rshunt The value of the external shunt used in milliohms
 * @param max_current The value of the maximum current in Amps
 *
 * @returns None
 */
void ina3221_config(int averages, int bus_time, int shunt_time, int rshunt, int max_current);

#endif /* PERIPHERALS_PWRMON_INA3221_H_ */
