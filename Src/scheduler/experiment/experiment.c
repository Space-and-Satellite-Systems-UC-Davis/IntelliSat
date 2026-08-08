#include "experiment.h"
#include "ADCS/ADCS.h"
#include "ADCS/virtual_rtos.h"
#include "scheduler/adcs_lock.h"
#include "FreeRTOS.h"
#include "task.h"

#define EXPERIMENT_PERIOD_MS (90UL * 60UL * 1000UL)

TaskHandle_t experimentHandle;

void experiment_task(void *pvParameters) {
    for (;;) {
        vi_reset_stale_timer();
        adcs_mode mode = ADCS_recommend_mode();
        adcs_main_status status;
        do {
            xSemaphoreTake(adcs_lock, portMAX_DELAY);
            status = ADCS_MAIN(mode);
            xSemaphoreGive(adcs_lock);
        } while (status == ADCS_MAIN_RESTARTED);
        vTaskDelay(pdMS_TO_TICKS(EXPERIMENT_PERIOD_MS));
    }
}
