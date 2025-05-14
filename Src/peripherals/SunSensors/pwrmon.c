#include <SunSensors/pwrmon.h>

INA226_I2C_Interface panel0_INA226, panel1_INA226, panel2_INA226, panel3_INA226;

INA226_I2C_Interface * panel0_I2C_interface = &panel0_INA226;
INA226_I2C_Interface * panel1_I2C_interface = &panel1_INA226;
INA226_I2C_Interface * panel2_I2C_interface = &panel2_INA226;
INA226_I2C_Interface * panel3_I2C_interface = &panel3_INA226;

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
void pwrmon_init(int averages, int bus_time, int shunt_time, int rshunt, int max_current) {
    panel0_I2C_interface->address = PWRMON_SENSOR_ADDRESS;
    panel0_I2C_interface->SCL_GPIO = PAN0_GPIO;
    panel0_I2C_interface->SCL_pin = PAN0_SCL_PIN;
    panel0_I2C_interface->SDA_GPIO = PAN0_GPIO;
    panel0_I2C_interface->SDA_pin = PAN0_SDA_PIN;

    panel1_I2C_interface->address = PWRMON_SENSOR_ADDRESS;
    panel1_I2C_interface->SCL_GPIO = PAN1_GPIO;
    panel1_I2C_interface->SCL_pin = PAN1_SCL_PIN;
    panel1_I2C_interface->SDA_GPIO = PAN1_GPIO;
    panel1_I2C_interface->SDA_pin = PAN1_SDA_PIN;

    panel2_I2C_interface->address = PWRMON_SENSOR_ADDRESS;
    panel2_I2C_interface->SCL_GPIO = PAN2_GPIO;
    panel2_I2C_interface->SCL_pin = PAN2_SCL_PIN;
    panel2_I2C_interface->SDA_GPIO = PAN2_GPIO;
    panel2_I2C_interface->SDA_pin = PAN2_SDA_PIN;

    panel3_I2C_interface->address = PWRMON_SENSOR_ADDRESS;
    panel3_I2C_interface->SCL_GPIO = PAN3_GPIO;
    panel3_I2C_interface->SCL_pin = PAN3_SCL_PIN;
    panel3_I2C_interface->SDA_GPIO = PAN3_GPIO;
    panel3_I2C_interface->SDA_pin = PAN3_SDA_PIN;

    INA226_init(panel0_I2C_interface, averages, bus_time, shunt_time, rshunt, max_current);
    INA226_init(panel1_I2C_interface, averages, bus_time, shunt_time, rshunt, max_current);
    INA226_init(panel2_I2C_interface, averages, bus_time, shunt_time, rshunt, max_current);
    INA226_init(panel3_I2C_interface, averages, bus_time, shunt_time, rshunt, max_current);
}

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
void pwrmon_config(int averages, int bus_time, int shunt_time, int rshunt, int max_current){
    INA226_config(panel0_I2C_interface, averages, bus_time, shunt_time, rshunt, max_current);
    INA226_config(panel1_I2C_interface, averages, bus_time, shunt_time, rshunt, max_current);
    INA226_config(panel2_I2C_interface, averages, bus_time, shunt_time, rshunt, max_current);
    INA226_config(panel3_I2C_interface, averages, bus_time, shunt_time, rshunt, max_current);
}


/**
 * Reads the shunt voltage from the sun sensor
 * @param gpio: i2c port for scl and sda
 * @param scl_pin: i2c scl pin
 * @param sda_pin: i2c sda_pin
 * 
 * @returns Voltage in mV 
 */
float pwrmon_getShuntVoltage(GPIO_TypeDef* gpio, int scl_pin, int sda_pin) {
    return INA226_getShuntVoltage(gpio, scl_pin, sda_pin, PWRMON_SENSOR_ADDRESS);
}


/**
 * Reads the bus voltage from the sun sensor
 * @param gpio: i2c port for scl and sda
 * @param scl_pin: i2c scl pin
 * @param sda_pin: i2c sda_pin
 * 
 * @returns Voltage in Volts
 */
float pwrmon_getBusVoltage(GPIO_TypeDef* gpio, int scl_pin, int sda_pin) {
    return INA226_getBusVoltage(gpio, scl_pin, sda_pin, PWRMON_SENSOR_ADDRESS);
}

/**
 * Reads the power from the sun sensor
 * @param gpio: i2c port for scl and sda
 * @param scl_pin: i2c scl pin
 * @param sda_pin: i2c sda_pin
 * 
 * @returns Wattage as a 16 bit integer
 */
float pwrmon_getPower(GPIO_TypeDef* gpio, int scl_pin, int sda_pin) {
    return INA226_getPower(gpio, scl_pin, sda_pin, PWRMON_SENSOR_ADDRESS);
}

/**
 * Reads the current from the sun sensor
 * @param gpio: i2c port for scl and sda
 * @param scl_pin: i2c scl pin
 * @param sda_pin: i2c sda_pin
 * 
 * @returns Amps as a 16 bit integer
 */
float pwrmon_getCurrent(GPIO_TypeDef* gpio, int scl_pin, int sda_pin) {
    return INA226_getCurrent(gpio, scl_pin, sda_pin, PWRMON_SENSOR_ADDRESS);
}

/**
 * Resets the mode of the sensors
 * @param mode The mode of the sensor (ontinuous, powered down)
 * 
 * @returns None
 */
void pwrmon_setMode(int mode) {
    INA226_setMode(panel0_I2C_interface, mode);
    INA226_setMode(panel1_I2C_interface, mode);
    INA226_setMode(panel2_I2C_interface, mode);
    INA226_setMode(panel3_I2C_interface, mode);
}


/**
 * Helper function to round up calculations to nearest number
 * @param num
 * 
 * @returns num rounded up
 */
int ceiling(float num);