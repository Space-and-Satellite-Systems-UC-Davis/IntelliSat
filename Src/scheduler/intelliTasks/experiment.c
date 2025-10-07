#include "intelliTasks_proto.h"
#include "peripherals/PDB/power_distribution.h"

//#include "ADCS.h"

bool experiment_time() {
    // Once every orbit or so (90m)
    return false;
}

void config_experiment() {
    // Logging is handled within the ADCS software
    pdb_hdd_on();
}

void experiment() {
    while (true)
        // Wait for start notification
        ulTaskNotifyTake(pdTrue, portMAX_DELAY);

        // Do not preempt
        taskENTER_CRITICAL();
        ADCS_MAIN(ADCS_recommend_mode());
        clean_experiment();
        taskEXIT_CRITICAL();
    }
}

void clean_experiment() {
    pdb_hdd_off();
}
