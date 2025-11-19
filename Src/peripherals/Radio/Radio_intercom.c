/*********************************************************
 * Radio_intercomm.h
 * 
 * Driver for communication with the Radio
 * 
 * Author: James Knepper (2025)
 * 
 * Log:
 *  12 November 2025 - Initial
 * 
 ********************************************************/

#include "Radio_intercom.h"
#include "MGTINTERCOM/mgt_intercom.h"

 /**
 * Initialize the radio USART ports
 */
void radio_init() {
    usart_init(RADIO_USART, PFC2Radio_BAUDRATE);
}

/**
 * Send data into the radio for transmission down to the ground
 * 
 * @param chunk  The data to be sent
 * @param nbytes  The size of the data to be sent.
 * 
 * @returns success or failure
 */
bool radio_push(uint8_t chunk[], size_t nbytes) {
    uint8_t header[2];
    header[0] = 'D';
    header[1] = ((nbytes - 1) / CHUNK_LENGTH) + 1;
    bool full_success = true;
    if (crc_transmit(RADIO_USART, header, 2)) {
        return crc_chunked_transmit(RADIO_USART, chunk, nbytes, ((nbytes - 1) / CHUNK_LENGTH) + 1);
    }
}

/**
 * Force the radio to send data to the PFC
 * 
 * @param chunk  A storage location for the data to be received
 * 
 * @returns number of bytes read and the data type
 */
RadioPacket radio_force_pull(uint8_t chunk[]) {
    RadioPacket fail = {-1, 'u'};
    if (!crc_transmit(RADIO_USART, "U", 1)) return fail;
    uint8_t buffer[MAX_PAYLOAD_BYTES];
    crc_read(RADIO_USART, buffer);
    return radio_pull(chunk, buffer[1], buffer[2]);
}

/**
 * Receive data from the radio. This should be called by a signal handler.
 * I assume the first packet has already been received and processed.
 * 
 * @param chunk  an empty storage vessel
 * 
 * @returns number of bytes read and the data type
 */
RadioPacket radio_pull(uint8_t chunk[], size_t nchunks, PFC2RadioMessageType datatype) {
    uint8_t subchunk[MAX_PAYLOAD_BYTES];
    int read = crc_chunked_read(RADIO_USART, chunk, CHUNK_LENGTH, nchunks);
    RadioPacket success;
    success.size = read;
    success.datatype = datatype;
    return success;
}

/**
 * Retrieve the radio's state
 * (t: transferring to earth, r: receiving from earth, o: neither)
 * 
 * @returns the state
 */
PFC2RadioState radio_get_state() {
    crc_transmit(RADIO_USART, "S", 1);
    PFC2RadioState buffer[MAX_PAYLOAD_BYTES];
    crc_read(RADIO_USART, buffer);
    return buffer[0];
}

/**
 * Receive the kill command from the ground. 
 */
void radio_killall() {
    if (mgt_killall()) {
        // TODO: Here Be Dragons
        // Kill the PFC
    }
}

bool radio_downlink(uint8_t chunk[], size_t nchunks) {
    uint8_t packet[2];
    packet[0] = TransferToGround;
    packet[1] = nchunks;
    crc_transmit(RADIO_USART, packet, 2);
}