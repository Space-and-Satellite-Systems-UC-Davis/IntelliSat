#include <ADC/adc.h>
#include "intelliTasks_proto.h"

#define LOW_BATTERY_THRESHOLD 20 // Example threshold

#define CHARGE_COMPLETE_THRESHOLD 80 // Example threshold

#define LOW_PWR_PRIORITY_THRESHOLD 2 // Example priority level.

// TODO Replace with actual percentage scale
#define ADC_TO_PERCENT_SCALE -5000

#define BATTERY_MONITOR_PIN -1

// ! Do not access the statics vars directly. Use their associated functions.

static bool low_pwr_mode = false; // The state repersenting whether or not in low power mode.

bool low_pwr_time();
void config_low_pwr();
void low_pwr();
void clean_low_pwr();
// * Below is optional but perhaps desirable
// enable_low_power_hardware();
// void restore_normal_hardware();
