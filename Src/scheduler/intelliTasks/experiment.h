#ifndef RTOS_EXPERIMENT_H
#define RTOS_EXPERIMENT_H

#include "intelliTasks_proto.h"

#define EXPERIMENT_DELAY 100 / portTICK_PERIOD_MS
#define EXPERIMENT_ON_EVENT_BIT (0x1 << 2)
#define EXPERIMENT_CLEAN_EVENT_BIT (0x1 << 3)

#endif//RTOS_EXPERIMENT_H
