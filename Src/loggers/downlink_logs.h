//
// Created by dilgill on 5/24/24.
//

#ifndef DOWNLINK_LOGS_H
#define DOWNLINK_LOGS_H

#include "experiment_logger.h"

#pragma pack(push,1)
struct DownlinkPacket {
    uint8_t packet_id;
    uint8_t type; // 0 = event, 1 = experiment
    uint8_t load[256];
    uint16_t checksum;
};
#pragma pack(pop)

// void logger_downlinkExpLogs(struct LocalExpLogs * local_exp_logs);
// void logger_downlinkEventLogs();

uint16_t compute_checksum(uint8_t* data, size_t len);
void downlink_experiment_logs_from_flash(uint32_t max_sectors);
void downlink_event_logs_from_flash(uint32_t max_sectors);
void logger_downlink_all(uint32_t max_exp_sectors, uint32_t max_event_sectors);

#endif //DOWNLINK_LOGS_H
