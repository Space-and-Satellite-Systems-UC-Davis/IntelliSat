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
    while (true) {
        // Wait for start notification
        ulTaskNotifyTake(pdTrue, portMAX_DELAY);

        // Do not preempt
        taskENTER_CRITICAL();
        ADCS_MAIN(ADCS_DETUMBLE);
        clean_detumble();
        taskEXIT_CRITICAL();
    }
}

void clean_detumble() {
    pdb_mgt_off();
}
