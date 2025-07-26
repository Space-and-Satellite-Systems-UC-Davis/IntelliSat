#include "intelliTasks_proto.h"
//#include "ADCS.h"

bool detumble_time() {
    //if time since last run > X minutes, return true
    return false;
}

void config_detumble() {
    //Enable coils board
    //Logging is handled within the ADCS software
}

void detumble() {
    while (true)
    {
        if (xEventGroupWaitBits(
                events_handler,
                (const EventBits_t)DETUMBLE_ON_EVENT_BIT,
                true,
                false,
                DETUMBLE_DELAY))
        {
            ADCS_MAIN(ADCS_DETUMBLE);
        }
        clean_detumble();
        vTaskDelay(LOW_PWR_DELAY); // Yield
    }
}

void clean_detumble() {
    //Disable coils board
}
