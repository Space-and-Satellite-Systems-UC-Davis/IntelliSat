/*
 * radio.c
 *
 *  Created on: Apr 16, 2024
 *      Author: ethansee
 */


#include "Radio/radio.h"

uint64_t lastStateChange = 0;

void radio_init() {
	usart_init(RADIO_USART, RADIO_BAUDRATE);
	lastStateChange = getSysTime();
}

bool radio_waitForAcknowledgement(uint64_t initialTime) {
	while (radio_readOneByte() != 'A') {
		if (getSysTime() - initialTime > RADIO_ACKNOWLEDGEMENT_TIMEOUT) {
			return false;
		}
	}
	return true;
}

void radio_sendMsg(char* message, ...) {
	usart_transmitBytes(RADIO_USART, message);
}

void radio_sendByte(uint8_t data) {
	usart_transmitChar(RADIO_USART, data);
}

bool radio_sendTransferToMemRequest(int numBytes) { //TODO: test
	radio_sendByte('T');
	radio_sendByte(numBytes);
	return radio_waitForAcknowledgement(getSysTime());

}

bool radio_transferToGroundStationRequest(uint8_t numBytesToSend, int numTimesToSend) {
	radio_sendByte('t');
	radio_sendByte(numBytesToSend);
	radio_sendByte(numTimesToSend);

	return radio_waitForAcknowledgement(getSysTime());
}

/**
 * Gets the current state of the radio as r = RX_ACTIVE, t = TX_ACTIVE, o = OFF
 * If timesout will return -1 as an error
 *
 * @return the state of the radio as a character
 */
char radio_receiveStateRequest(void){
	radio_sendByte('R');

	char byteRead = -1;

	uint64_t initRequestTime = getSysTime();
	while (byteRead != 'r' && byteRead != 't' && byteRead != 'o') {

		if (getSysTime() - initRequestTime > RADIO_ACKNOWLEDGEMENT_TIMEOUT) {
			return -1;
		}

		byteRead = radio_readOneByte();
	}

	radio_sendByte('A');

	return byteRead;
}

void radio_sendByteStream(int numberOfBytes, uint8_t buffer[]) {
	uint64_t startingStreamTime = getSysTime();
	for (int i = 0; i < numberOfBytes; i++) {
			//If byte per second past threshold then halt byte sending
			if (i / ((startingStreamTime - getSysTime()) / 1000) > BYTE_PER_SECOND_LIMIT) {
				i--;
			} else {
				radio_sendByte(buffer[i]);
			}
	}
}


/**
 * Sends a stream of bytes
 *
 * @param numberOfBytes: Number of bytes to send
 * @param buffer: The buffer with the bytes to send
 *
 * @return False if sending failed and true otherwise
 */
bool radio_sendByteStreamToMemRequest(int numberOfBytes, uint8_t buffer[]) { //TODO: test
	uint64_t startingStreamTime = getSysTime();
	if (!radio_sendTransferToMemRequest(numberOfBytes)) {
		return false;
	}
	radio_sendByteStream(numberOfBytes, buffer);
	return true;
}

void radio_sendState(enum RadioState state) {
	if (getSysTime() - lastStateChange > RADIO_MAX_STATE_CHANGE_TIME_LIMIT) {
		switch (state) {
			case RADIO_OFF:
				radio_sendByte('o');
				break;
			case RADIO_TX_ACTIVE:
				radio_sendByte('t');
				break;
			case RADIO_RX_ACTIVE:
				radio_sendByte('r');
				break;
		}
		lastStateChange = getSysTime();
	}

}


/**
 * Reads bytes from the radio into a buffer
 *
 * @param buffer: the buffer that will be filled by the radio's data
 * @param receive_buffer_size: the number of bytes the buffer will take
 */
void radio_readBytes(uint8_t buffer[], int receive_buffer_size) {
	uint16_t amount_to_recieve = receive_buffer_size;
	uint16_t amount_read = 0;

	uint64_t initTime  = getSysTime();
	while (amount_read < amount_to_recieve) {

			if (getSysTime() - initTime > 10) {
					break;
			}

			amount_read += usart_recieveBytes(RADIO_USART,
					buffer,
					(receive_buffer_size - amount_read));
			buffer += amount_read;	// pointer arithmetic :)

			if (usart_recieverTimedOut(RADIO_USART)) {
				break;
			}
	}
	amount_read = usart_recieveBytes(RADIO_USART, buffer, receive_buffer_size);

}

/**
 * Reads one byte from the buffer and returns it
 */
int radio_readOneByte() {
	uint8_t buffer[1];
	radio_readBytes(buffer, 1);
	return buffer[0];

}
