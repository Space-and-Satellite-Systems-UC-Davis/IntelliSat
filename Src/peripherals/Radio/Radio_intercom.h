/*********************************************************
 * Radio_intercomm.h
 * 
 * Driver for communication with the Radio
 * 
 * Author: James Knepper (2025)
 * 
 * Log:
 *  6 November 2025 - Initial
 * 
 ********************************************************/

#ifndef __RADIO_INTERCOM__
#define __RADIO_INTERCOM__

#include "globals.h"
#include "UART/crc.h"
#include "ADCS/logging_records.h"

#define RADIO_USART USART1
#define PFC2Radio_BAUDRATE 9600
#define CHUNK_LENGTH 50
/* Peer reserves whole chunks and rejects >= 30 chunks, even when empty. */
#define RADIO_MAX_DOWNLINK_BYTES (CHUNK_LENGTH * 29)
#define MAX_UINT8_T 255

typedef enum {
    DownloadData = 'D', //Receive data from PFC
    UploadData = 'U', //Transfer data to PFC
	SendState = 'S', //Send state to PFC
	TransferToGround = 'T', //Transfer data to ground station
    KillAll = 'K', // Kill the satelite
} PFC2RadioMessageType;

//Tells PFC what type of data is being sent
//For future flags, try to keep them lowercase and don't step on other enums
typedef enum {
    Unflagged = 'u', //No upload flags were designed yet
} PFC2RadioUploadFlag;

typedef enum {
    RadioStateUnknown = 0, //No validated state response (local error, not a wire value)
    TXactive = 't', //Busy tranceiving data to ground station
	RXactive = 'r', //Busy receiving data from ground station
    Idle = 'o', //Not busy
} PFC2RadioState;

typedef struct {
    size_t size;
    uint8_t datatype;
} RadioPacket;

/**
 * Initialize the radio USART ports
 */
void radio_init();

/**
 * Send data into the radio for transmission down to the ground
 * 
 * @param chunk  The data to be sent
 * @param nbytes  The size of the data to be sent.
 * 
 * @returns success or failure
 */
bool radio_push(uint8_t chunk[], size_t nbytes);

/**
 * Force the radio to send data to the PFC
 * 
 * @param chunk  A storage location for the data to be received
 * @param capacity  Size of that storage in bytes; oversized headers are rejected
 * 
 * @returns number of bytes read and the data type
 */
RadioPacket radio_force_pull(uint8_t chunk[], size_t capacity);

/**
 * Receive data from the radio. This should be called by a signal handler.
 * I assume the first packet has already been received and processed.
 * 
 * @param chunk  An output array with at least nbytes bytes of storage
 * 
 * @returns number of bytes read and the data type
 */
RadioPacket radio_pull(uint8_t chunk[], size_t nbytes, PFC2RadioMessageType datatype);

/**
 * Retrieve the radio's state
 * (t: transferring to earth, r: receiving from earth, o: neither)
 * 
 * @returns the validated state, or RadioStateUnknown on communication failure
 */
PFC2RadioState radio_get_state();

/**
 * Receive the kill command from the ground. 
 */
void radio_killall();

/**
 * Order Radio to send a given amount of memory in the buffer to be transmitted
 * down to the ground.
 * NOTE: radio_push() your data into memory first, THEN call radio_downlink().
 *
 * @param chunk  The data to be sent
 * @param nbytes  The size of the data to be sent.
 *
 * @returns success or failure
 */
bool radio_downlink(uint8_t chunk[], size_t nchunks);

/**
 * Sends the idle log to the radio to be sent via radio to our home radio.
 * The idle log is filled inside the function at the time of calling.
 *
 * @returns success or failure
 */
bool radio_downlink_idle_log();

/**
 * @param chunk An array to store the uplink data
 * @param capacity Size of that array in bytes
 *
 * @returns Actual byte count for a complete valid transfer, or -1 on failure.
 * Failure does not establish a completed transfer or an empty peer; partial
 * output must not be used. Uses the existing CRC ACKs, not an application ACK.
 */
int radio_uplink(uint8_t chunk[], size_t capacity);


void radio_getLastUplink(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *hour, uint8_t *minute, uint8_t *second);

void echo();

void initEmptyChunk(uint8_t chunk[]);

#endif /* __RADIO_INTERCOM__ */
