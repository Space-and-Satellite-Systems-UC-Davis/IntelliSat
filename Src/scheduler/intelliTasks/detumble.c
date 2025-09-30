#include "intelliTasks_proto.h"
#include "peripherals/PDB/power_distribution.h"

//#include "ADCS.h"

bool detumble_time() {
    //if time since last run > X minutes, return true
    return false;
}

void config_detumble() {
    //Logging is handled within the ADCS software
    pdb_mgt_on();
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
    pdb_mgt_off();
}
