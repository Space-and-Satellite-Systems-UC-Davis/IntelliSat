#include "INA226.h"

/*************************** Initialize the 4 sensors *************************/
Sun_Sensor Sensor_0 = {0, Pan0_GPIO, Pan0_GPIO, Pan0_SDA_PIN, Pan0_SCL_PIN};
Sun_Sensor Sensor_1 = {1, Pan1_GPIO, Pan1_GPIO, Pan1_SDA_PIN, Pan1_SCL_PIN};
Sun_Sensor Sensor_2 = {2, Pan2_GPIO, Pan2_GPIO, Pan2_SDA_PIN, Pan2_SCL_PIN};
Sun_Sensor Sensor_3 = {3, Pan3_GPIO, Pan3_GPIO, Pan3_SDA_PIN, Pan3_SCL_PIN};


/*************************** Parameters *************************/
//static uint16_t mag_magFullScale = 0;
static Sun_Sensor *current_selected = Sensor_0;