#include "intelliTasks_proto.h"

//#include "ADCS.h"

bool experiment_time() {
    // Once every orbit or so (90m)
    return false;
}

void config_experiment() {
    // Enable HDD
    // Logging is handled within the ADCS software
}

void experiment() {
    while (true)
    {
        if (xEventGroupWaitBits(
                events_handler,
                (const EventBits_t)EXPERIMENT_ON_EVENT_BIT,
                true,
                false,
                EXPERIMENT_DELAY))
        {
            ADCS_MAIN(ADCS_recommend_mode());
        }
        clean_experiment();
        vTaskDelay(EXPERIMENT_DELAY); // Yield
    }
}

void clean_experiment() {
    // Disable HDD
}
