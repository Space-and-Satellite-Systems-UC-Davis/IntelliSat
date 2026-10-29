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
#include "print_scan.h"
#include "Radio/log_record.h"

bool talking;

/**
 * Initialize the radio USART ports
 */
void radio_init() {
    usart_init(RADIO_USART, PFC2Radio_BAUDRATE);
    talking = false;
}

/**
 * 
 */
bool are_talking() {
    if (talking) return true;
    setCommunicatingRadio(true);
    bool talking = crc_transmit(RADIO_USART, "?", 1);
    setCommunicatingRadio(false);
    return talking;
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
    if (!chunk || nbytes == 0 || nbytes > RADIO_MAX_DOWNLINK_BYTES) return false;
    if (!are_talking()) return false;
    setCommunicatingRadio(true);    
    uint8_t header[3];
    header[0] = 'D';
    header[1] = ((nbytes - 1) / CHUNK_LENGTH) + 1;
    header[2] = nbytes % CHUNK_LENGTH; //remainder, to calculate total num bytes
    if(header[2] == 0){
        header[2] = CHUNK_LENGTH;
    }
    if (crc_transmit(RADIO_USART, header, 3)) {
        bool transmitted = crc_chunked_transmit(RADIO_USART, chunk, nbytes, CHUNK_LENGTH);
        setCommunicatingRadio(false);
        return transmitted;
    }
    setCommunicatingRadio(false);
    return false;
}

/**
 * Force the radio to send data to the PFC
 * 
 * @param chunk  A storage location for the data to be received
 * 
 * @returns number of bytes read and the data type
 */
RadioPacket radio_force_pull(uint8_t chunk[], size_t capacity) {
    RadioPacket result = {(size_t)-1, 'u'};
    if (!chunk || capacity == 0 || !are_talking()) return result;
    setCommunicatingRadio(true);
    uint8_t buffer[MAX_PAYLOAD_BYTES];
    if (crc_transmit(RADIO_USART, "U", 1) &&
        crc_read(RADIO_USART, buffer) == 4 &&
        buffer[0] == 'U' && buffer[1] != 0 &&
        buffer[2] > 0 && buffer[2] <= CHUNK_LENGTH && buffer[3] == Unflagged) {
        size_t total = (size_t)(buffer[1] - 1) * CHUNK_LENGTH + buffer[2];
        if (total <= capacity) result = radio_pull(chunk, total, buffer[3]);
    }
    setCommunicatingRadio(false);
    return result;
}

/**
 * Receive data from the radio. This should be called by a signal handler.
 * I assume the first packet has already been received and processed.
 * 
 * @param chunk  an empty storage vessel
 * 
 * @returns number of bytes read and the data type
 */
RadioPacket radio_pull(uint8_t chunk[], size_t nbytes, PFC2RadioMessageType datatype) {
    RadioPacket fail;
    fail.size = 0;
    fail.datatype = 0;
    if (!chunk || nbytes == 0 || nbytes > (size_t)CHUNK_LENGTH * MAX_UINT8_T)
        return fail;
    if (!are_talking()) return fail;
    int nchunks = (nbytes-1)/CHUNK_LENGTH + 1;
    setCommunicatingRadio(true);
    int received = crc_chunked_read(RADIO_USART, chunk, nchunks, nbytes);
    setCommunicatingRadio(false);
    if (received < 0 || (size_t)received != nbytes) return fail;
    RadioPacket success;
    success.size = (size_t)received;
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
    if (!are_talking()) return RadioStateUnknown;
    setCommunicatingRadio(true);
    PFC2RadioState state = RadioStateUnknown;
    uint8_t buffer[MAX_PAYLOAD_BYTES];
    if (crc_transmit(RADIO_USART, "S", 1) &&
        crc_read(RADIO_USART, buffer) == 1 &&
        (buffer[0] == TXactive || buffer[0] == RXactive || buffer[0] == Idle)) {
        state = (PFC2RadioState)buffer[0];
    }
    setCommunicatingRadio(false);
    return state;
}

/**
 * Receive the kill command from the ground. 
 */
void radio_killall() {
    if (!are_talking()) return;
    if (mgt_killall()) {
        // TODO: Here Be Dragons
        // Kill the PFC
    }
}

bool radio_downlink(uint8_t chunk[], size_t nchunks) {
    if (!are_talking()) return false;
    uint8_t packet[2];
    packet[0] = TransferToGround;
    packet[1] = nchunks;
    setCommunicatingRadio(true);
    bool transmitted = crc_transmit(RADIO_USART, packet, 2);
    setCommunicatingRadio(false);
    return transmitted;
}


int radio_uplink(uint8_t chunk[], size_t capacity) {
    if (!chunk || capacity == 0) return -1;
    setCommunicatingRadio(true);
    uint8_t header[MAX_PAYLOAD_BYTES];
    int size = crc_read(RADIO_USART, header);
    int i = 0;
    while (i < size && header[i] == 0) ++i; /* Preserve PR #121's zero padding. */
    int received = -1;
    if (size - i == 4 && header[i] == 'U' && header[i + 1] != 0 &&
        header[i + 2] > 0 && header[i + 2] <= CHUNK_LENGTH &&
        header[i + 3] == Unflagged) {
        int nchunks = header[i + 1];
        size_t total = (size_t)(nchunks - 1) * CHUNK_LENGTH + header[i + 2];
        if (total <= capacity) {
            int actual = crc_chunked_read(RADIO_USART, chunk, nchunks, total);
            if (actual >= 0 && (size_t)actual == total) received = actual;
        }
    }
    setCommunicatingRadio(false);
    return received;
}

void echo() {
    if (!are_talking()) return;
    uint8_t packet[MAX_MESSAGE_BYTES];
    setCommunicatingRadio(true);
    size_t bytes = usart_receiveBytes(RADIO_USART, packet, MAX_MESSAGE_BYTES);
    setCommunicatingRadio(false);
    printMsg("Radio Says: <%s>", packet);
}

bool radio_downlink_idle_log() {
	log_record_idle log = fill_log_idle();
	uint8_t* log_bytes = (uint8_t*) &log;
	bool success = radio_push(log_bytes, sizeof(log));
	if (!success) return false;

	// radio_push to stuff what we need into memory. Downlink later.
	success = radio_downlink(NULL, sizeof(log));
	return success;
}




void initEmptyChunk(uint8_t chunk[]) {
	for (size_t i = 0; i < CHUNK_LENGTH; i++) {
		chunk[i] = 0;
	}
}



void radio_getLastUplink(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *hour, uint8_t *minute, uint8_t *second){
    //TODO
    //lalalalalala does something here 
}
