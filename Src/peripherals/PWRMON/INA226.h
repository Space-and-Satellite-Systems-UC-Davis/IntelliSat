/*
* INA226.h (Sun sensor interface)
*
*
*/

#include <I2C/i2c.h>


//Address
#define SENSOR_ADDRESS 0x45
#define CONFIG_REGISTER 0
#define CAL_REGISTER 5

//Macros for the Sunsensor pins
#define PAN0_GPIO GPIOG
#define PAN0_SDA_PIN 14
#define PAN0_SCL_PIN 13
#define PAN1_GPIO GPIOF
#define PAN1_SDA_PIN 11
#define PAN1_SCL_PIN 2
#define PAN2_GPIO GPIOG
#define PAN2_SDA_PIN 0
#define PAN2_SCL_PIN 1
#define PAN3_GPIO GPIOG
#define PAN3_SDA_PIN 4
#define PAN3_SCL_PIN 5

//Battery monitor pins
#define BATMON_GPIO GPIOH
#define BATMON_SDA 0
#define BATMON_SCL 1

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
 * Initializes the sun sensor with I2c, and calls config function
 *
 * @param averages Number of samples to collect and average
 * @param bus_time The conversion time for bus voltage measurements
 * @param shunt_time The conversion time for shunt voltage measurements
 * @param rshunt The value of the external shunt used in milliohms
 * @param max_current The value of the maximum current
 *
 * @returns None
 */
void pwrmon_init(int averages, int bus_time, int shunt_time, int rshunt, int max_current);

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
void pwrmon_config(int averages, int bus_time, int shunt_time, int rshunt, int max_current);


/**
 * Reads the shunt voltage from the sun sensor
 * @param gpio: i2c port for scl and sda
 * @param scl_pin: i2c scl pin
 * @param sda_pin: i2c sda_pin
 * 
 * @returns Voltage in mV 
 */
float pwrmon_getShuntVoltage(GPIO_TypeDef* gpio, int scl_pin, int sda_pin);


/**
 * Reads the bus voltage from the sun sensor
 * @param gpio: i2c port for scl and sda
 * @param scl_pin: i2c scl pin
 * @param sda_pin: i2c sda_pin
 * 
 * @returns Voltage in Volts
 */
float pwrmon_getBusVoltage(GPIO_TypeDef* gpio, int scl_pin, int sda_pin);

/**
 * Reads the power from the sun sensor
 * @param gpio: i2c port for scl and sda
 * @param scl_pin: i2c scl pin
 * @param sda_pin: i2c sda_pin
 * 
 * @returns Wattage as a 16 bit integer
 */
float pwrmon_getPower(GPIO_TypeDef* gpio, int scl_pin, int sda_pin);

/**
 * Reads the current from the sun sensor
 * @param gpio: i2c port for scl and sda
 * @param scl_pin: i2c scl pin
 * @param sda_pin: i2c sda_pin
 * 
 * @returns Amps as a 16 bit integer
 */
float pwrmon_getCurrent(GPIO_TypeDef* gpio, int scl_pin, int sda_pin);

/**
 * Resets the mode of the sensors
 * @param mode The mode of the sensor (ontinuous, powered down)
 * 
 * @returns None
 */
void pwrmon_setMode(int mode);


/**
 * Helper function to round up calculations to nearest number
 * @param num
 * 
 * @returns num rounded up
 */
int ceiling(float num);
