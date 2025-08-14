//
// Created by dilgill on 5/24/24.
// Edited by Keanu Chau on 6/20/2025
//

#include "downlink_logs.h"

#include "event_logger.h"
#include "experiment_logger.h"
#include "logger_to_flash.h"
#include "print_scan.h"

#include <stdio.h>
#include <string.h>

extern struct LocalEventLogs local_event_logs;
extern struct LocalExpLogs local_exp_logs;

/*
ERROR DETECTION: Sent along with the packet. Receiver recomputes the checksum from the received data and compares it to the one sent.
Calculate a simple 16-bit checksum over the given byte array.
Inputs:
	- data: Pointer to the byte array to checksum
	- len: Number of bytes in the array
Returns:
	- 16-bit integer representing the checksum
*/
uint16_t compute_checksum(uint8_t* data, size_t len) {
    uint16_t sum = 0;
    for (size_t i = 0; i < len; ++i) {
        sum += data[i];
    }
    return sum;
}

// Fetch ExperimentLogs from FLASH, package each into a DownlinkPacket, and print it.

void downlink_experiment_logs_from_flash(uint32_t max_sectors) {
    uint8_t sector_buffer[FLASH_SECTOR_SIZE];
    uint16_t packet_id = 0;
    uint32_t sectors_processed = 0;
    uint32_t last_downlinked_sector = FLASH_header.exp1_header.start_sector_num;

    // Calculate logs per sector based on actual struct size
    const int logs_per_sector = FLASH_SECTOR_SIZE / sizeof(struct ExperimentLog);

	// Loop through each sector in the experiment log
    for (uint32_t sector = FLASH_header.exp1_header.start_sector_num;
         sector < FLASH_header.exp1_header.tail && sectors_processed < max_sectors; ++sector) {
        // Read sector from FLASH
        if (!flash_readSector(sector, sector_buffer)) {
            printMsg("Failed to read sector %u\n\r", sector);
            continue;
        }

        sectors_processed++;
        last_downlinked_sector = sector;

		// Loop through logs in this sector
        for (int i = 0; i < logs_per_sector; i++) {
            struct ExperimentLog* log = (struct ExperimentLog*)&sector_buffer[i * sizeof(struct ExperimentLog)];
			// Create DownlinkPacket
            struct DownlinkPacket packet = {
                .packet_id = packet_id++,
                .type = 1  // Experiment
            };

            // Zero the entire load
            memset(packet.load, 0, sizeof(packet.load));

            // Copy the log data
            memcpy(packet.load, log, sizeof(struct ExperimentLog));
            packet.checksum = compute_checksum(packet.load, sizeof(packet.load));

            // Push to radio functions (placeholder for now)
            // TODO: Replace with actual radio transmission
            printMsg("DL Packet ID: %u Type: %u Checksum: %04X\n\r", packet.packet_id, packet.type, packet.checksum);
            for (int j = 0; j < 256; j++) {
                printMsg("%02X ", packet.load[j]);
                if ((j+1) % 16 == 0) printMsg("\n\r");
            }
        }
    }

    // Update header to reflect the latest downlinked sector
    if (sectors_processed > 0) {
        FLASH_header.exp1_header.start_sector_num = last_downlinked_sector + 1;
        logger_pushHeader();
    }
}

void downlink_event_logs_from_flash(uint32_t max_sectors) {
    uint8_t sector_buffer[FLASH_SECTOR_SIZE];
    uint16_t packet_id = 0;
    uint32_t sectors_processed = 0;
    uint32_t last_downlinked_sector = FLASH_header.events_header.start_sector_num;

    // Calculate logs per sector based on actual struct size
    const int logs_per_sector = FLASH_SECTOR_SIZE / sizeof(struct EventLog);

    for (uint32_t sector = FLASH_header.events_header.start_sector_num;
         sector < FLASH_header.events_header.tail && sectors_processed < max_sectors; ++sector) {

        if (!flash_readSector(sector, sector_buffer)) {
            printMsg("Failed to read event sector %u\n\r", sector);
            continue;
        }

        sectors_processed++;
        last_downlinked_sector = sector;

        for (int i = 0; i < logs_per_sector; i++) {
            struct EventLog* log = (struct EventLog*)&sector_buffer[i * sizeof(struct EventLog)];
            struct DownlinkPacket packet = {
                .packet_id = packet_id++,
                .type = 0  // Event
            };

            // Zero the entire load
            memset(packet.load, 0, sizeof(packet.load));

            memcpy(packet.load, log, sizeof(struct EventLog));
            packet.checksum = compute_checksum(packet.load, sizeof(packet.load));

            // Push to radio functions (placeholder for now)
            // TODO: Replace with actual radio transmission
            printMsg("DL Packet ID: %u Type: %u Checksum: %04X\n\r", packet.packet_id, packet.type, packet.checksum);
            for (int j = 0; j < 256; j++) {
                printMsg("%02X ", packet.load[j]);
                if ((j+1) % 16 == 0) printMsg("\n\r");
            }
        }
    }

    // Update header to reflect the latest downlinked sector
    if (sectors_processed > 0) {
        FLASH_header.events_header.start_sector_num = last_downlinked_sector + 1;
        logger_pushHeader();
    }
}

void logger_downlink_all(uint32_t max_exp_sectors, uint32_t max_event_sectors) {
    printMsg("\n\r==== DOWNLINK EVENT LOGS ====\n\r");
    downlink_event_logs_from_flash(max_event_sectors);

    printMsg("\n\r==== DOWNLINK EXPERIMENT LOGS ====\n\r");
    downlink_experiment_logs_from_flash(max_exp_sectors);
}

///////////////////////// TEST VERSION /////////////////////////////

// void logger_downlinkExpLogs(struct LocalExpLogs * local_exp_logs) {
//     printf("Downlinking Exp logs\r\n");
//     for(uint16_t i = 0; i < local_exp_logs->num_logs; ++i) {
//         struct ExperimentLog* log = &local_exp_logs->logs[i];

//         if (i % 4 == 0) {
//         	printMsg("\n\r");
//         }
//         printMsg("exp_num: %u ", log->exp_num);


// 		// Radio transmission
// 		// Error Correction ?


//         /*
//         printMsg("exp_num: %u\r\nrtc_time: %u\r\n",
//             log->exp_num,
//             log->rtc_time
//         );

//         printMsg("gyro_x: %d\r\ngyro_y: %d\r\ngyro_z: %d\r\n",
// 			log->gyro_x,
// 			log->gyro_y,
// 			log->gyro_z
// 		);

//         printMsg("mag_x: %d\r\nmag_y: %d\r\nmag_z: %d\r\n",
// 			log->mag_x,
// 			log->mag_y,
// 			log->mag_z
// 		);

//         printMsg("ss_1: %u\r\nss_2: %u\r\nss_3: %u\r\n",
// 			log->sunsensor_1,
// 			log->sunsensor_2,
// 			log->sunsensor_3
// 		);

//         printMsg("ss_4: %u\r\nss_5: %u\r\nss_6: %u\r\n\r\n",
// 			log->sunsensor_4,
// 			log->sunsensor_5,
// 			log->sunsensor_6
// 		);
// 		*/
//     }
// }

// void logger_downlinkEventLogs() {
//     printf("In downlink event logs\r\n");
// }
