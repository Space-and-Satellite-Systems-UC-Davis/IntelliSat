#include "INA226.h"

/*************************** Parameters *************************/
static Sun_Sensor current_sensor = {0, PAN0_GPIO, PAN0_GPIO, PAN0_SDA_PIN, PAN0_SCL_PIN, MODE_POWERED_DOWN};
static int current_lsb; //in microamps
static int config;

int celing(float num){
    return num - (int)num > 0 ? (int)num + 1: (int)num;
}

void sensor_init(int averages, int bus_time, int shunt_time, int mode, int rshunt, int max_current){
    for(int i = 0; i<4; i++){
        softi2c_init(current_sensor.SCL_GPIO, current_sensor.SCL_PIN, current_sensor.SDA_GPIO, current_sensor.SDA_PIN);
        current_sensor.mode = mode;
    }
    sensor_config(averages, bus_time, shunt_time, mode, rshunt, max_current);
}

void sensor_config(int averages, int bus_time, int shunt_time, int mode, int rshunt, int max_current){
    config = averages << 9 | bus_time << 6 | shunt_time << 3 | mode;
    current_lsb = celing(max_current / 32768.0 * MICRO); //2^15
    int cal = celing(0.00512 / (current_lsb/MICRO * rshunt/MILLI)); //round to nearest integer above
    for(int i = 0; i<4; i++){
        softi2c_writeReg16(current_sensor.SCL_GPIO, current_sensor.SCL_PIN, current_sensor.SDA_GPIO, current_sensor.SDA_PIN, SENSOR_ADDRESS, 0, config);
        softi2c_delay();
        softi2c_writeReg16(current_sensor.SCL_GPIO, current_sensor.SCL_PIN, current_sensor.SDA_GPIO, current_sensor.SDA_PIN, SENSOR_ADDRESS, 5, cal);
        softi2c_delay();
        softi2c_writeReg16(current_sensor.SCL_GPIO, current_sensor.SCL_PIN, current_sensor.SDA_GPIO, current_sensor.SDA_PIN, SENSOR_ADDRESS, 6, 1 << 3); //set conversion ready flag
        softi2c_delay();
    }
}

void trigger(){
    bool powered = current_sensor.mode == MODE_POWERED_DOWN;

    set_mode(MODE_TRIGGERED, false);
    int ready = 0;
    while(!ready)
    {
        ready = softi2c_readReg(current_sensor.SCL_GPIO, current_sensor.SCL_PIN, current_sensor.SDA_GPIO, current_sensor.SDA_PIN, SENSOR_ADDRESS, 6);
        ready = (ready >> 3) & 1;
    }
    if(powered)
        set_mode(MODE_POWERED_DOWN, false);
}

float get_shunt_voltage(){
    int output = softi2c_readReg(current_sensor.SCL_GPIO,current_sensor.SCL_PIN, current_sensor.SDA_GPIO, current_sensor.SDA_PIN, SENSOR_ADDRESS, 0x01);
    return output * .0025;
}


float get_bus_voltage(){
    int output = softi2c_readReg(current_sensor.SCL_GPIO,current_sensor.SCL_PIN, current_sensor.SDA_GPIO, current_sensor.SDA_PIN, SENSOR_ADDRESS, 0x02);
    return output * 1.25/MILLI; 
}


float get_power(){
    int output = softi2c_readReg(current_sensor.SCL_GPIO,current_sensor.SCL_PIN, current_sensor.SDA_GPIO, current_sensor.SDA_PIN, SENSOR_ADDRESS, 0x03);
    return output * current_lsb/(double)MICRO * 25; 
}


float get_current(){
    int output = softi2c_readReg(current_sensor.SCL_GPIO,current_sensor.SCL_PIN, current_sensor.SDA_GPIO, current_sensor.SDA_PIN, SENSOR_ADDRESS, 0x04);
    return output * current_lsb / (double)MICRO;
}


void set_mode(int mode, bool all){
    int changed_config = (config & 7) | mode;
    if(all)
    {
        for(int i = 0; i < 4; i++){
            softi2c_writeReg(current_sensor.SCL_GPIO, current_sensor.SCL_PIN, current_sensor.SDA_GPIO, current_sensor.SDA_PIN, SENSOR_ADDRESS, 0, changed_config);
            current_sensor.mode = mode;
        }
    }else{
        softi2c_writeReg(current_sensor.SCL_GPIO, current_sensor.SCL_PIN, current_sensor.SDA_GPIO, current_sensor.SDA_PIN, SENSOR_ADDRESS, 0, changed_config);
        current_sensor.mode = mode;
    }
}


void select_sensor(int sensor){
    if(sensor < 0 || sensor > 3)
        sensor = 0;
    
    current_sensor.ID = sensor;
    switch (sensor) {
        case 0: 
            current_sensor.SCL_GPIO = PAN0_GPIO;
            current_sensor.SCL_PIN = PAN0_SCL_PIN;
            current_sensor.SDA_GPIO = PAN0_GPIO;
            current_sensor.SDA_PIN = PAN0_SDA_PIN;
            break;
        case 1:
            current_sensor.SCL_GPIO = PAN1_GPIO;
            current_sensor.SCL_PIN = PAN1_SCL_PIN;
            current_sensor.SDA_GPIO = PAN1_GPIO;
            current_sensor.SDA_PIN = PAN1_SDA_PIN;
            break;
        case 2: 
            current_sensor.SCL_GPIO = PAN2_GPIO;
            current_sensor.SCL_PIN = PAN2_SCL_PIN;
            current_sensor.SDA_GPIO = PAN2_GPIO;
            current_sensor.SDA_PIN = PAN2_SDA_PIN;
            break;
        case 3: 
            current_sensor.SCL_GPIO = PAN3_GPIO;
            current_sensor.SCL_PIN = PAN3_SCL_PIN;
            current_sensor.SDA_GPIO = PAN3_GPIO;
            current_sensor.SDA_PIN = PAN3_SDA_PIN;
            break;
    }
}


int get_id(){

    int output = 0;
	int hex_value = 0x40;
	while (hex_value < 0x50)
	{
		output = softi2c_readReg16(current_sensor.SCL_GPIO, current_sensor.SCL_PIN, current_sensor.SDA_GPIO, current_sensor.SDA_PIN, hex_value, 0xfe);
		if (output != 0)
			break;
		hex_value +=1;
	}
    //int output = softi2c_readReg(current_sensor.SCL_GPIO, current_sensor.SCL_PIN, current_sensor.SDA_GPIO, current_sensor.SDA_PIN, SENSOR_ADDRESS, 0xfe);
    return output;
}
