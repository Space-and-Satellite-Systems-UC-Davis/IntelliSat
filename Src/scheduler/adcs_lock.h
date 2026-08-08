#ifndef ADCS_LOCK_H_
#define ADCS_LOCK_H_

#include "FreeRTOS.h"
#include "semphr.h"

extern SemaphoreHandle_t adcs_lock;

void adcs_lock_init(void);

#endif // ADCS_LOCK_H_
