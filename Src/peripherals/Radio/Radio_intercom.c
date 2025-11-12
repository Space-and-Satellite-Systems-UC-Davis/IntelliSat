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
 * @param nbytes  The size of the data to be sent
 * 
 * @returns success or failure
 */
bool radio_push(uint8_t chunk[], size_t nbytes) {
    
}

/**
 * Receive data from the radio
 * 
 * @param chunk  an empty storage vessel
 * 
 * @returns number of bytes read
 */
size_t radio_pull(uint8_t chunk[]);

/**
 * Retrieve the radio's state
 * (t: transferring to earth, r: receiving from earth, o: neither)
 * 
 * @returns the state
 */
PFC2RadioState radio_get_state();

/**
 * Receive the kill command from the ground
 */
void radio_killall();

bool downlink(uint8_t chunk[], size_t nbytes);