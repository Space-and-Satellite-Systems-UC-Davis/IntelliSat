/*
* INA226.h (Sun sensor interface)
*
*
*/

#include <I2C/i2c.h>

//Address
#define CONFIG_REGISTER 0
#define CAL_REGISTER 5

// general struct for containing I2C protocol ports and pins
typedef struct {
    GPIO_TypeDef * SCL_GPIO;
    int SCL_pin;
    GPIO_TypeDef * SDA_GPIO;
    int SDA_pin;
    int address;
} INA226_I2C_Interface;

//number of averages
#define AVERAGE_1   0
#define AVERAGE_4   1
#define AVERAGE_16  2
#define AVERAGE_64  3
#define AVERAGE_128 4
#define AVERAGE_256 5
#define AVERAGE_512 6

//conversion times in microseconds
#define CONVERT_140     0
#define CONVERT_204     1
#define CONVERT_332     2
#define CONVERT_588     3
#define CONVERT_1100    4
#define CONVERT_2116    5
#define CONVERT_4156    6
#define CONVERT_8244    7

//modes
#define MODE_CONTINUOUS     7
#define MODE_TRIGGERED      3 
#define MODE_POWERED_DOWN   0 

//number conversions
#define MICRO 1000000
#define MILLI 1000

/**
 * Initialize I2C and call config function
 *
 * @param interface A pointer to a struct containing the address, ports and pins of the INA226
 * @param averages Number of samples to collect and average
 * @param bus_time The conversion time for bus voltage measurements
 * @param shunt_time The conversion time for shunt voltage measurements
 * @param rshunt The value of the external shunt used in milliohms
 * @param max_current The value of the maximum current
 *
 * @returns None
 */
void INA226_init(INA226_I2C_Interface * interface, int averages, int bus_time, int shunt_time, int rshunt, int max_current);

/**
 * Initial configuration of the config register and calibration register
 *
 * @param interface A pointer to a struct containing the address, ports and pins of the INA226
 * @param averages Number of samples to collect and average
 * @param bus_time The conversion time for bus voltage measurements
 * @param shunt_time The conversion time for shunt voltage measurements
 * @param rshunt The value of the external shunt used in milliohms
 * @param max_current The value of the maximum current in Amps
 *
 * @returns None
 */
void INA226_config(INA226_I2C_Interface * interface, int averages, int bus_time, int shunt_time, int rshunt, int max_current);


/**
 * Reads the shunt voltage
 * @param gpio: i2c port for scl and sda
 * @param scl_pin: i2c scl pin
 * @param sda_pin: i2c sda_pin
 * @param address: i2c address
 * 
 * @returns Voltage in mV 
 */
float INA226_getShuntVoltage(GPIO_TypeDef* gpio, int scl_pin, int sda_pin, int address);


/**
 * Reads the bus voltage
 * @param gpio: i2c port for scl and sda
 * @param scl_pin: i2c scl pin
 * @param sda_pin: i2c sda_pin
 * @param address: i2c address
 * 
 * @returns Voltage in Volts
 */
float INA226_getBusVoltage(GPIO_TypeDef* gpio, int scl_pin, int sda_pin, int address);

/**
 * Reads the power
 * @param gpio: i2c port for scl and sda
 * @param scl_pin: i2c scl pin
 * @param sda_pin: i2c sda_pin
 * @param address: i2c address
 * 
 * @returns Wattage as a 16 bit integer
 */
float INA226_getPower(GPIO_TypeDef* gpio, int scl_pin, int sda_pin, int address);

/**
 * Reads the current
 * @param gpio: i2c port for scl and sda
 * @param scl_pin: i2c scl pin
 * @param sda_pin: i2c sda_pin
 * @param address: i2c address
 * 
 * @returns Amps as a 16 bit integer
 */
float INA226_getCurrent(GPIO_TypeDef* gpio, int scl_pin, int sda_pin, int address);

/**
 * Resets the mode of the sensor
 * @param interface A pointer to a struct containing the address, ports and pins of the INA226
 * @param mode The mode of the sensor (ontinuous, powered down)
 * 
 * @returns None
 */
void INA226_setMode(INA226_I2C_Interface * interface, int mode);


/**
 * Helper function to round up calculations to nearest number
 * @param num
 * 
 * @returns num rounded up
 */
int ceiling(float num);

