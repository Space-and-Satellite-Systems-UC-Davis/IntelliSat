/*
* INA226.h (Sun sensor interface)
*
*
*/

#include <I2C/i2c.h>

typedef struct
{
    /* data */
    int ID;
    GPIO_TypeDef* SDA_GPIO;
    GPIO_TypeDef* SCL_GPIO;
    int SDA_PIN;
    int SCL_PIN;
}Sun_Sensor;

//Macros for the Sunsensor pins
#define Pan0_GPIO GPIOG
#define Pan0_SDA_PIN 14
#define Pan0_SCL_PIN 13
#define Pan1_GPIO GPIOF
#define Pan1_SDA_PIN 11
#define Pan1_SCL_PIN 2
#define Pan2_GPIO GPIOG
#define Pan2_SDA_PIN 0
#define Pan2_SCL_PIN 1
#define Pan3_GPIO GPIOG
#define Pan3_SDA_PIN 4
#define Pan3_SCL_PIN 5



