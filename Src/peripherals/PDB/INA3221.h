/*
 * INA3221.h
 *
 *  Created on: Apr 2, 2026
 */

#ifndef INA3221_H_
#define INA3221_H_

#include <I2C/i2c.h>

//Address
// TODO: THESE ARE COPIED, GET REAL VALUES
//Address
// Possible either 0x40 or 0x41 address
#define SENSOR_ADDRESS 0x40
//#define SENSOR_ADDRESS 0x41

//#define SENSOR_ADDRESS 0x42
//#define SENSOR_ADDRESS 0x43

//#define SENSOR_ADDRESS 0x6
//#define SENSOR_ADDRESS 0x7

//#define SENSOR_ADDRESS 0x0

#define CONFIG_REGISTER 0
#define CAL_REGISTER 5

//Macros for the INA3221 pins
#define INA3221_GPIO GPIOF
#define INA3221_SDA_PIN 14
#define INA3221_SCL_PIN 15

#define MODE_CONTINUOUS     7

//number conversions
#define MICRO 1000000
#define MILLI 1000


/**
 * Initializes PDB voltage monitor with I2c, and calls config function
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
