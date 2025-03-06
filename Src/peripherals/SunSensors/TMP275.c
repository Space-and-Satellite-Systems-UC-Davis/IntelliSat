#include "TMP275.h"

static int resolution = 0;
static float resolution_conversion = 0;
static int initialConfig = 0;




void temp_sensor_init(int res){
    int config = res << 5;
    initialConfig = config;

    softi2c_writeReg16(PAN0_GPIO, PAN0_SCL_PIN, PAN0_GPIO, PAN0_SDA_PIN, TMP0_ADDRESS, 1, config);
    softi2c_writeReg16(PAN0_GPIO, PAN0_SCL_PIN, PAN0_GPIO, PAN0_SDA_PIN, TMP1_ADDRESS, 1, config);
    
    softi2c_writeReg16(PAN1_GPIO, PAN1_SCL_PIN, PAN1_GPIO, PAN1_SDA_PIN, TMP0_ADDRESS, 1, config);
    softi2c_writeReg16(PAN1_GPIO, PAN1_SCL_PIN, PAN1_GPIO, PAN1_SDA_PIN, TMP1_ADDRESS, 1, config);

    softi2c_writeReg16(PAN2_GPIO, PAN2_SCL_PIN, PAN2_GPIO, PAN2_SDA_PIN, TMP0_ADDRESS, 1, config);
    softi2c_writeReg16(PAN2_GPIO, PAN2_SCL_PIN, PAN2_GPIO, PAN2_SDA_PIN, TMP1_ADDRESS, 1, config);
    
    softi2c_writeReg16(PAN3_GPIO, PAN3_SCL_PIN, PAN3_GPIO, PAN3_SDA_PIN, TMP0_ADDRESS, 1, config);
    softi2c_writeReg16(PAN3_GPIO, PAN3_SCL_PIN, PAN3_GPIO, PAN3_SDA_PIN, TMP1_ADDRESS, 1, config);
    
    switch (res) {
        case RESOLUTION_9:
            resolution = 9;
            resolution_conversion = RESOLUTION_CONVERSION_9;
            break;
        case RESOLUTION_10:
            resolution = 10;
            resolution_conversion = RESOLUTION_CONVERSION_10;
            break;
        case RESOLUTION_11:
            resolution = 11;
            resolution_conversion = RESOLUTION_CONVERSION_11;
            break;
        case RESOLUTION_12:
            resolution = 12;
            resolution_conversion = RESOLUTION_CONVERSION_12;
            break;
    }
}


void shutdown(bool shutdown){

        int config = (initialConfig & ~3) | shutdown;
        softi2c_writeReg16(PAN0_GPIO, PAN0_SCL_PIN, PAN0_GPIO, PAN0_SDA_PIN, TMP0_ADDRESS, 1, config);
        softi2c_writeReg16(PAN0_GPIO, PAN0_SCL_PIN, PAN0_GPIO, PAN0_SDA_PIN, TMP1_ADDRESS, 1, config);
        
        softi2c_writeReg16(PAN1_GPIO, PAN1_SCL_PIN, PAN1_GPIO, PAN1_SDA_PIN, TMP0_ADDRESS, 1, config);
        softi2c_writeReg16(PAN1_GPIO, PAN1_SCL_PIN, PAN1_GPIO, PAN1_SDA_PIN, TMP1_ADDRESS, 1, config);

        softi2c_writeReg16(PAN2_GPIO, PAN2_SCL_PIN, PAN2_GPIO, PAN2_SDA_PIN, TMP0_ADDRESS, 1, config);
        softi2c_writeReg16(PAN2_GPIO, PAN2_SCL_PIN, PAN2_GPIO, PAN2_SDA_PIN, TMP1_ADDRESS, 1, config);
        
        softi2c_writeReg16(PAN3_GPIO, PAN3_SCL_PIN, PAN3_GPIO, PAN3_SDA_PIN, TMP0_ADDRESS, 1, config);
        softi2c_writeReg16(PAN3_GPIO, PAN3_SCL_PIN, PAN3_GPIO, PAN3_SDA_PIN, TMP1_ADDRESS, 1, config);
}




float get_temp(GPIO_TypeDef* gpio, int scl_pin, int sda_pin, int address){
    int16_t temp = softi2c_readReg16(gpio, scl_pin, gpio, sda_pin, address, 0);
    return (temp >> (16-resolution))* resolution_conversion;
   
}
