#ifndef RTOS_DETUMBLE_H
#define RTOS_DETUMBLE_H

#include "intelliTasks_proto.h"

#define DETUMBLE_DELAY 100 / portTICK_PERIOD_MS
#define DETUMBLE_ON_EVENT_BIT (0x1 << 2)
#define DETUMBLE_CLEAN_EVENT_BIT (0x1 << 3)

#endif//RTOS_DETUMBLE_H
