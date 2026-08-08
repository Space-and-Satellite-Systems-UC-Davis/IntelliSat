#include "adcs_lock.h"

SemaphoreHandle_t adcs_lock;

void adcs_lock_init(void) {
    adcs_lock = xSemaphoreCreateMutex();
    configASSERT(adcs_lock != NULL);
}
