#include "intelliTasks_proto.h"
#include "Radio/Radio_intercom.h"

/**
 * FreeRTOS task for uplink/downlink
 * 
 * @param args  Args passed to the RTOS task handle
 * 
 * @returns void
 */
static void comms(void *args) {

    // Receive uplink
    uint8_t chunk[MAX_PAYLOAD_BYTES];
    radio_force_pull(chunk);
    RadioPacket* data = (RadioPacket*) chunk;

    // If killall stop immediately
    if (data->datatype == 'K') { // Condition to check if uplink is kill command
        radio_killall();
    }

    // Optionally log uplink here
    
    // Loop load radio with downlink data
    uint8_t downlinkData[MAX_PAYLOAD_BYTES];
    while (false) {
        // TODO: Replace condition with check for additional data to pull
        
        size_t nChunks = 0;
        // Load downlinkData with logs to downlink
        // Some storage retrieval call passing downlinkData
        // Make sure it returns the number of bytes
        // getLogFromStorage(downlinkData, &nChunks);

        if (!radio_downlink(downlinkData, nChunks)) {
            break;
        }
    }

    // Force downlink after radio transmit buffer is loaded
    if (radio_push(NULL, 0)) {
        // Case: Transmit success

    } else {
        // Case: Transmit failure

    }
}
