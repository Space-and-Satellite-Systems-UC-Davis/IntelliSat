/*
 * INA3221.h
 *
 *  Created on: Apr 2, 2026
 */

#ifndef INA3221_H_
#define INA3221_H_

#include <I2C/i2c.h>
#include "globals.h"

//Battery INA address
#define INA3221_ADDR_0 0x40
#define INA3221_ADDR_BATTERY 0x40
// The other one
#define INA3221_ADDR_1 0x41

#define INA_CVRF_MSK 0x1 << 0
// Do not uncomment or create unused registers defs
typedef enum INA3221_REG_ENUM {
    CONFIG_REG = 0x00,
    // CH1_SHUNT_REG = 0x01,
    CH1_BUS_REG = 0x02,
    // CH2_SHUNT_REG = 0x03,
    CH2_BUS_REG = 0x04,
    // CH3_SHUNT_REG = 0x05,
    CH3_BUS_REG = 0x06,
    BATTERY_BUS_REG = 0x06, //Dupe for readability
    MASK_ENABLE_REG = 0x0F,
    FAB_ID_REG = 0xFE,
    DIE_ID_REG = 0xFF
} INA3221_REG;

#define INA_CFG_MODE_MSK 0x7 << 0
typedef enum INA3221_MODE_ENUM {
    BUS_SINGLE_SHOT = 0x3,
    SHUNT_CONTINUOUS = 0x5,
    SHUNT_BUS_CONTINUOUS = 0x7

} INA3221_MODE;

//number of averages
typedef enum INA3221_AVG_ENUM {
    AVERAGE_1 = 0,
    AVERAGE_4,
    AVERAGE_16,
    AVERAGE_64,
    AVERAGE_128,
    AVERAGE_256,
    AVERAGE_512
} INA3221_AVG;

//conversion times in microseconds
typedef enum INA3221_CONVERT_ENUM {
    CONVERT_140 = 0,
    CONVERT_204,
    CONVERT_332,
    CONVERT_588,
    CONVERT_1100,
    CONVERT_2116,
    CONVERT_4156,
    CONVERT_8244
} INA3221_CONVERT_ENUM;

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
 * @param mode The mode of the sensor (continuous, single-shot)
 *
 * @returns None
 */
void ina3221_init(int averages, int bus_time, int shunt_time, uint8_t mode);

/**
 * Initial configuration of the sun sensor's config register and calibration register
 *
 * @param averages Number of samples to collect and average
 * @param bus_time The conversion time for bus voltage measurements
 * @param shunt_time The conversion time for shunt voltage measurements
 * @param mode The mode of the sensor (continuous, single-shot)
 *
 * @returns None
 */
void ina3221_config(int averages, int bus_time, int shunt_time, uint8_t mode);

/**
 * Reads the shunt voltage from the sun sensor
 * @param address: Address for 1 of 2 monitors
 * @param channel: Channel index for 1 of 3 channels
 * @param scl_port: i2c port for scl
 * @param scl_pin: i2c scl pin
 * @param sda_port: i2c port sda
 * @param sda_pin: i2c sda_pin
 *
 * @returns Voltage in mV
 */
float ina3221_getBusVoltage(uint8_t address, INA3221_REG channel,
		GPIO_TypeDef* scl_port, int scl_pin, 
        GPIO_TypeDef* sda_port, int sda_pin);

/**
 * Resets the mode of the sensors
 * @param mode The mode of the sensor (continuous, powered down)
 *
 * @returns None
 */
void ina3221_setMode(uint8_t mode);

/**
 * Reads die ID of monitor
 * NOTE: Expect to receive 0x3220
 * 
 * @param address: Address for 1 of 2 monitors
 * @param scl_port: i2c port for scl
 * @param scl_pin: i2c scl pin
 * @param sda_port: i2c port sda
 * @param sda_pin: i2c sda_pin
 * @returns die ID
 */
uint16_t ina3221_getDieID(uint8_t address,
    GPIO_TypeDef* scl_port, int scl_pin, GPIO_TypeDef* sda_port, int sda_pin);

#endif /* PERIPHERALS_PWRMON_INA3221_H_ */
