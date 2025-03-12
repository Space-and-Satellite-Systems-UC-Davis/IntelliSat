#include <I2C/i2c.h>


#define TMP0_ADDRESS 0x49 //1001001
#define TMP1_ADDRESS 0x4A //1001010


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


#define RESOLUTION_9 0
#define RESOLUTION_10 1
#define RESOLUTION_11 2
#define RESOLUTION_12 3


#define RESOLUTION_CONVERSION_9 0.5
#define RESOLUTION_CONVERSION_10 0.25
#define RESOLUTION_CONVERSION_11 0.125
#define RESOLUTION_CONVERSION_12 0.0625


#define SHUT_DOWN_MODE 1
#define CONTINUOUS_MODE 0



/**
 * Initializes I2C communication and configures temp sensor with preferred resolution
 *
 * @param resolution Resolution of the internal analog-to-digital converter
 *
 * @return None
 *
*/
void temp_sensor_init(int resolution);


/**
 * Puts the temp sensor into shut down or continuous mode
 * Can still make measurements, will shut down again after measurement taken
 *
 * @param shutdown Whether you want to shutdown (true) or back into continuous mode(false)
 *
 *  @return None
 */
void shutdown(bool shutdown);


/**
 * Reads the temperature recorded by the temp sensor
 * @param gpio: i2c port for scl and sda
 * @param scl_pin: i2c scl pin
 * @param sda_pin: i2c sda_pin
 * @param address: Address of temp sensor (TMP0_ADDRESS or TMP1_ADDRESS) 
 * 
 * @return temperature in Celsius
 *
 */
float get_temp(GPIO_TypeDef* gpio, int scl_pin, int sda_pin, int address);


