#include "SunSensors/sun_sensors.h"
#include <globals.h>
#include <print_scan.h>
#include <Buttons/buttons.h>
#include <math.h>

#define V0 0.05

const float normal_vectors[6][3] = {
    { 1.0f,  0.0f,  0.0f}, 
    {-1.0f,  0.0f,  0.0f},
    { 0.0f,  1.0f,  0.0f}, 
    { 0.0f, -1.0f,  0.0f}, 
    { 0.0f,  0.0f,  1.0f}, 
    { 0.0f,  0.0f, -1.0f}  
};

void normalize_readings(float voltages[6])
{
  for(int i = 0; i < 6; i++)
  {
    voltages[i] = voltages[i]/V0;
  }
}

void calculate_sun_vector(const float voltages[6], float sun_vector[3])
{
  float vx = (voltages[0] > voltages[1]) ? voltages[0] : -voltages[1];
  float vy = (voltages[2] > voltages[3]) ? voltages[2] : -voltages[3];
  float vz = (voltages[4] > voltages[5]) ? voltages[4] : -voltages[5];

  sun_vector[0] = vx;
  sun_vector[1] = vy;
  sun_vector[2] = vz;

  float sum_of_squares = vx * vx + vy * vy + vz * vz;
  float mag = sqrtf(sum_of_squares);

  for(int i = 0; i < 3; i ++)
  {    
    if (mag > 1e-6) {
        sun_vector[i] = sun_vector[i]/mag;
    } else {
        sun_vector[i] = 0.0f;
    }
  }
}

void testFunction_Sun_Sensor_Main(){

    float readings[6] = {0,0,0,0,0,0};

    float sun_vector[3] = {0.0,0.0,0.0};
    while (1) {
        if (get_buttonStatus_SW1()) {
            //for testing purposes: this will get readings from each diode on 3 panels
            //prints for panel 0 (side face)
            readings[0] = sun_sensors_readVoltage(PANEL0, DIODE0);
            printMsg("Voltage 1: %f\r\n", readings[0]);
            readings[1] = sun_sensors_readVoltage(PANEL0, DIODE1);
            printMsg("Voltage 2: %f\r\n", readings[1]);

            calculate_sun_vector(readings,sun_vector);
            printMsg("Predicted sun vector: [%f, %f, %f]\r\n", sun_vector[0], sun_vector[1], sun_vector[2]);
        }
    }
}