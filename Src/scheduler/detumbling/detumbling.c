#include "detumbling.h"
#include "ADCS/ADCS.h"
#include "ADCS/virtual_rtos.h"
#include "scheduler/adcs_lock.h"
#include "FreeRTOS.h"
#include "task.h"

#define DETUMBLE_PERIOD_MS (60UL * 60UL * 1000UL)

TaskHandle_t detumbleHandle;

void detumble_task(void *pvParameters) {
    for (;;) {
        vi_reset_stale_timer();
        adcs_main_status status;
        do {
            xSemaphoreTake(adcs_lock, portMAX_DELAY);
            status = ADCS_MAIN(ADCS_DETUMBLE);
            xSemaphoreGive(adcs_lock);
        } while (status == ADCS_MAIN_RESTARTED);
        vTaskDelay(pdMS_TO_TICKS(DETUMBLE_PERIOD_MS));
    }
}
