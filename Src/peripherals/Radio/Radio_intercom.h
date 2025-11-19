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

#define RADIO_USART USART1
#define PFC2Radio_BAUDRATE 9600
#define CHUNK_LENGTH 8
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
 * 
 * @returns number of bytes read and the data type
 */
RadioPacket radio_force_pull(uint8_t chunk[]);

/**
 * Receive data from the radio. This should be called by a signal handler.
 * I assume the first packet has already been received and processed.
 * 
 * @param chunk  an empty storage vessel
 * 
 * @returns number of bytes read and the data type
 */
RadioPacket radio_pull(uint8_t chunk[], size_t nchunks, PFC2RadioMessageType datatype);

/**
 * Retrieve the radio's state
 * (t: transferring to earth, r: receiving from earth, o: neither)
 * 
 * @returns the state
 */
PFC2RadioState radio_get_state();

/**
 * Receive the kill command from the ground. 
 */
void radio_killall();

bool radio_downlink(uint8_t chunk[], size_t nchunks);

#endif /* __RADIO_INTERCOM__ */