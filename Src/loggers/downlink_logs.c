//
// Created by dilgill on 5/24/24.
//

#include "downlink_logs.h"

#include "event_logger.h"
#include "experiment_logger.h"

#include <stdio.h>

extern struct LocalEventLogs local_event_logs;
extern struct LocalExpLogs local_exp_logs;

void logger_downlinkExpLogs(struct LocalExpLogs * local_exp_logs) {
    printf("Downlinking Exp logs\r\n");
    for(uint16_t i = 0; i < local_exp_logs->num_logs; ++i) {
        struct ExperimentLog* log = &local_exp_logs->logs[i];

        printMsg("exp_num: %u\r\nrtc_time: %u\r\n",
            log->exp_num,
            log->rtc_time
        );

        printMsg("gyro_x: %d\r\ngyro_y: %d\r\ngyro_z: %d\r\n",
			log->gyro_x,
			log->gyro_y,
			log->gyro_z
		);

        printMsg("mag_x: %d\r\nmag_y: %d\r\nmag_z: %d\r\n",
			log->mag_x,
			log->mag_y,
			log->mag_z
		);

        printMsg("ss_1: %u\r\nss_2: %u\r\nss_3: %u\r\n",
			log->sunsensor_1,
			log->sunsensor_2,
			log->sunsensor_3
		);

        printMsg("ss_4: %u\r\nss_5: %u\r\nss_6: %u\r\n\r\n",
			log->sunsensor_4,
			log->sunsensor_5,
			log->sunsensor_6
		);
    }
}

void logger_downlinkEventLogs() {
    printf("In downlink event logs\r\n");
}
