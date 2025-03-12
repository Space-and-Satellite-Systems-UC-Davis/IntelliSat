#ifndef LOW_PWR_H
#define LOW_PWR_H

#include "intelliTasks_proto.h"

// Notification bits
#define LOW_PWR_NOTIFY_ENTER (1 << 0)
#define LOW_PWR_NOTIFY_EXIT  (1 << 1)
#define LOW_PWR_PRIORITY_THRESHOLD 2

// Hardware thresholds (adjust based on IC specifications)
#define LOW_BATTERY_THRESHOLD      20
#define BATT_HYSTERESIS            5

// Public API
bool low_pwr_time(void);
void config_low_pwr(void);
void low_pwr(void);
void clean_low_pwr(void);

// Hardware control
void enable_low_power_hardware(void);
void restore_normal_hardware(void);

#endif // LOW_PWR_H