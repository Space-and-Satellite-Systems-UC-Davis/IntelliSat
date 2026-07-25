#include "INA226.h"

/*************************** Parameters *************************/
static int current_lsb; //in microamps
static int16_t config;

int ceiling(float num){
    return num - (int)num > 0 ? (int)num + 1: (int)num;
}

void pwrmon_init(int averages, int bus_time, int shunt_time, int rshunt, int max_current){

    softi2c_init(PAN0_GPIO, PAN0_SCL_PIN, PAN0_GPIO, PAN0_SDA_PIN);
    softi2c_init(PAN1_GPIO, PAN1_SCL_PIN, PAN1_GPIO, PAN1_SDA_PIN);
    softi2c_init(PAN2_GPIO, PAN2_SCL_PIN, PAN2_GPIO, PAN2_SDA_PIN);
    softi2c_init(PAN3_GPIO, PAN3_SCL_PIN, PAN3_GPIO, PAN3_SDA_PIN);
    softi2c_init(BATMON_GPIO, BATMON_SCL, BATMON_GPIO, BATMON_SDA);

    pwrmon_config(averages, bus_time, shunt_time, rshunt, max_current);
}

void pwrmon_config(int averages, int bus_time, int shunt_time, int rshunt, int max_current){
    config = averages << 9 | bus_time << 6 | shunt_time << 3 | MODE_CONTINUOUS;
    current_lsb = ceiling(max_current / 32768.0 * MICRO); //2^15
    int16_t cal = ceiling((0.00512 / (current_lsb * 1.0 /MICRO)) /(rshunt * 1.0 /MILLI)); //round to nearest integer above

    //sensor0
    softi2c_writeReg16(PAN0_GPIO, PAN0_SCL_PIN, PAN0_GPIO, PAN0_SDA_PIN, SENSOR_ADDRESS, CONFIG_REGISTER, config); // configuration register
    softi2c_writeReg16(PAN0_GPIO, PAN0_SCL_PIN, PAN0_GPIO, PAN0_SDA_PIN, SENSOR_ADDRESS, CAL_REGISTER, cal); //calibration register
    //sensor1
    softi2c_writeReg16(PAN1_GPIO, PAN1_SCL_PIN, PAN1_GPIO, PAN1_SDA_PIN, SENSOR_ADDRESS, CONFIG_REGISTER, config);
    softi2c_writeReg16(PAN1_GPIO, PAN1_SCL_PIN, PAN1_GPIO, PAN1_SDA_PIN, SENSOR_ADDRESS, CAL_REGISTER, cal);
    //sensor2
    softi2c_writeReg16(PAN2_GPIO, PAN2_SCL_PIN, PAN2_GPIO, PAN2_SDA_PIN, SENSOR_ADDRESS, CONFIG_REGISTER, config);
    softi2c_writeReg16(PAN2_GPIO, PAN2_SCL_PIN, PAN2_GPIO, PAN2_SDA_PIN, SENSOR_ADDRESS, CAL_REGISTER, cal);
    //sensor3
    softi2c_writeReg16(PAN3_GPIO, PAN3_SCL_PIN, PAN3_GPIO, PAN3_SDA_PIN, SENSOR_ADDRESS, CONFIG_REGISTER, config);
    softi2c_writeReg16(PAN3_GPIO, PAN3_SCL_PIN, PAN3_GPIO, PAN3_SDA_PIN, SENSOR_ADDRESS, CAL_REGISTER, cal);

    //battery monitor
    softi2c_writeReg16(BATMON_GPIO, BATMON_SCL, BATMON_GPIO, BATMON_SDA, SENSOR_ADDRESS, CONFIG_REGISTER, config);
    softi2c_writeReg16(BATMON_GPIO, BATMON_SCL, BATMON_GPIO, BATMON_SDA, SENSOR_ADDRESS, CAL_REGISTER, cal);

}

//no need to trigger, only use continuous mode or power-down mode
/*void trigger(){
    bool powered = current_sensor.mode == MODE_POWERED_DOWN;
    set_mode(MODE_TRIGGERED, false);
    int ready = 0;
    while(!ready)
    {
        ready = softi2c_readReg16(current_sensor.SCL_GPIO, current_sensor.SCL_PIN, current_sensor.SDA_GPIO, current_sensor.SDA_PIN, SENSOR_ADDRESS, 6);
        ready = (ready >> 3) & 1;
    }
    if(powered)
        set_mode(MODE_POWERED_DOWN, false);
}*/

float pwrmon_getShuntVoltage(GPIO_TypeDef* gpio, int scl_pin, int sda_pin){
    int16_t output = softi2c_readReg16(gpio,scl_pin, gpio, sda_pin, SENSOR_ADDRESS, 0x01);

    return output * .0025;
}


float pwrmon_getBusVoltage(GPIO_TypeDef* gpio, int scl_pin, int sda_pin){
    int16_t output = softi2c_readReg16(gpio,scl_pin, gpio, sda_pin, SENSOR_ADDRESS, 0x02);
    return output * 1.25/MILLI; 
}


float pwrmon_getPower(GPIO_TypeDef* gpio, int scl_pin, int sda_pin){
    int16_t output = softi2c_readReg16(gpio,scl_pin, gpio, sda_pin, SENSOR_ADDRESS, 0x03);
    return output * current_lsb/(double)MICRO * 25; 
}


float pwrmon_getCurrent(GPIO_TypeDef* gpio, int scl_pin, int sda_pin){
    int16_t output = softi2c_readReg16(gpio,scl_pin, gpio, sda_pin, SENSOR_ADDRESS, 0x04);
    return output * current_lsb / (double)MICRO;
}


void pwrmon_setMode(int mode){
    int16_t changed_config = (config & ~7) | mode;

    softi2c_writeReg16(PAN0_GPIO, PAN0_SCL_PIN, PAN0_GPIO, PAN0_SDA_PIN, SENSOR_ADDRESS, 0, changed_config);
    softi2c_writeReg16(PAN1_GPIO, PAN1_SCL_PIN, PAN1_GPIO, PAN1_SDA_PIN, SENSOR_ADDRESS, 0, changed_config);
    softi2c_writeReg16(PAN2_GPIO, PAN2_SCL_PIN, PAN2_GPIO, PAN1_SDA_PIN, SENSOR_ADDRESS, 0, changed_config);
    softi2c_writeReg16(PAN3_GPIO, PAN3_SCL_PIN, PAN3_GPIO, PAN3_SDA_PIN, SENSOR_ADDRESS, 0, changed_config);
    
}


