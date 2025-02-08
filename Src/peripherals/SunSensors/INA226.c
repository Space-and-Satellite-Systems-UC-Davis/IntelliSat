#include "INA226.h"

/*************************** Initialize the 4 sensors *************************/
static Sun_Sensor sensor_0 = {0, PAN0_GPIO, PAN0_GPIO, PAN0_SDA_PIN, PAN0_SCL_PIN, MODE_POWERED_DOWN};
static Sun_Sensor sensor_1 = {1, PAN1_GPIO, PAN1_GPIO, PAN1_SDA_PIN, PAN1_SCL_PIN, MODE_POWERED_DOWN};
static Sun_Sensor sensor_2 = {2, PAN2_GPIO, PAN2_GPIO, PAN2_SDA_PIN, PAN2_SCL_PIN, MODE_POWERED_DOWN};
static Sun_Sensor sensor_3 = {3, PAN3_GPIO, PAN3_GPIO, PAN3_SDA_PIN, PAN3_SCL_PIN, MODE_POWERED_DOWN};
static Sun_Sensor* sensors[4] = {&sensor_0, &sensor_1, &sensor_2, &sensor_3};


/*************************** Parameters *************************/
static int current_sensor = 0;
static int current_lsb; //in microamps
static int config;

int ceil(float num){
    return num - (int)num > 0 ? (int)num + 1: (int)num;
}

void sensor_init(int averages, int bus_time, int shunt_time, int mode, int rshunt, int max_current){
    for(int i = 0; i<4; i++){
        softi2c_init(sensors[i]->SCL_GPIO, sensors[i]->SCL_PIN, sensors[i]->SDA_GPIO, sensors[i]->SDA_PIN);
        sensors[i]->mode = mode;
    }
    sensor_config(averages, bus_time, shunt_time, mode, rshunt, max_current);
}

void sensor_config(int averages, int bus_time, int shunt_time, int mode, int rshunt, int max_current){
    config = averages << 9 | bus_time << 6 | shunt_time << 3 | mode;
    current_lsb = ceil(max_current / 32768.0 * MICRO); //2^15
    int cal = ceil(0.00512 / (current_lsb/MICRO * rshunt/MILLI)); //round to nearest integer above
    for(int i = 0; i<4; i++){
        softi2c_writeReg(sensors[i]->SCL_GPIO, sensors[i]->SCL_PIN, sensors[i]->SDA_GPIO, sensors[i]->SDA_PIN, SENSOR_ADDRESS, 0, config);
        softi2c_writeReg(sensors[i]->SCL_GPIO, sensors[i]->SCL_PIN, sensors[i]->SDA_GPIO, sensors[i]->SDA_PIN, SENSOR_ADDRESS, 5, cal);
        softi2c_writeReg(sensors[i]->SCL_GPIO, sensors[i]->SCL_PIN, sensors[i]->SDA_GPIO, sensors[i]->SDA_PIN, SENSOR_ADDRESS, 6, 1 << 3); //set conversion ready flag
    }
}

void trigger(){
    bool powered = sensors[current_sensor]->mode == MODE_POWERED_DOWN;

    set_mode(MODE_TRIGGERED, false);
    int ready = 0;
    while(!ready)
    {
        ready = softi2c_readReg(sensors[current_sensor]->SCL_GPIO, sensors[current_sensor]->SCL_PIN, sensors[current_sensor]->SDA_GPIO, sensors[current_sensor]->SDA_PIN, SENSOR_ADDRESS, 6);
        ready = (ready >> 3) & 1;
    }
    if(powered)
        set_mode(MODE_POWERED_DOWN, false);
}

float get_shunt_voltage(){
    int output = softi2c_readReg(sensors[current_sensor]->SCL_GPIO,sensors[current_sensor]->SCL_PIN, sensors[current_sensor]->SDA_GPIO, sensors[current_sensor]->SDA_PIN, SENSOR_ADDRESS, 0x01);
    return output * .0025;
}


float get_bus_voltage(){
    int output = softi2c_readReg(sensors[current_sensor]->SCL_GPIO,sensors[current_sensor]->SCL_PIN, sensors[current_sensor]->SDA_GPIO, sensors[current_sensor]->SDA_PIN, SENSOR_ADDRESS, 0x02);
    return output * 1.25/MILLI; 
}


float get_power(){
    int output = softi2c_readReg(sensors[current_sensor]->SCL_GPIO,sensors[current_sensor]->SCL_PIN, sensors[current_sensor]->SDA_GPIO, sensors[current_sensor]->SDA_PIN, SENSOR_ADDRESS, 0x03);
    return output * current_lsb/(double)MICRO * 25; 
}


float get_current(){
    int output = softi2c_readReg(sensors[current_sensor]->SCL_GPIO,sensors[current_sensor]->SCL_PIN, sensors[current_sensor]->SDA_GPIO, sensors[current_sensor]->SDA_PIN, SENSOR_ADDRESS, 0x04);
    return output * current_lsb / (double)MICRO;
}


void set_mode(int mode, bool all){
    int changed_config = config & 7 | mode;
    if(all)
    {
        for(int i = 0; i < 4; i++){
            softi2c_writeReg(sensors[i]->SCL_GPIO, sensors[i]->SCL_PIN, sensors[i]->SDA_GPIO, sensors[i]->SDA_PIN, SENSOR_ADDRESS, 0, changed_config);
            sensors[i]->mode = mode;
        }
    }else{
        softi2c_writeReg(sensors[current_sensor]->SCL_GPIO, sensors[current_sensor]->SCL_PIN, sensors[current_sensor]->SDA_GPIO, sensors[current_sensor]->SDA_PIN, SENSOR_ADDRESS, 0, changed_config);
        sensors[current_sensor]->mode = mode;
    }
}


void select_sensor(int sensor){
    if(sensor < 0 || sensor > 3)
        sensor = 0;
    else
        current_sensor = sensor;
}


int get_id(){
    return softi2c_readReg(sensor_0.SCL_GPIO, sensor_0.SCL_PIN, sensor_0.SDA_GPIO, sensor_0.SDA_PIN, SENSOR_ADDRESS, 0xff );
}
