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

void radio_sendMsg(char* message, ...) {
	usart_transmitBytes(RADIO_USART, message);
}

void radio_sendByte(uint8_t data) {
	usart_transmitChar(RADIO_USART, data);

}

void radio_sendByteStream(int numberOfBytes, uint8_t buffer[]) {
	uint64_t startingStreamTime = getSysTime();
	int bufferAllocatedSize = sizeof(buffer) / sizeof(buffer[0]);

	if (bufferAllocatedSize < numberOfBytes) {
		return;
	}

	for (int i = 0; i < numberOfBytes; i++) {
		//If byte per second past threshold then halt byte sending
		if (i / ((startingStreamTime - getSysTime()) / 1000) > BYTE_PER_SECOND_LIMIT) {
			i--;
		} else {
			radio_sendByte(buffer[i]);
		}
	}
}

void radio_sendState(enum RadioState state) {
	if (getSysTime() - lastStateChange > RADIO_MAX_STATE_CHANGE_TIME_LIMIT) {
		switch (state) {
			case RADIO_STANDBY:
				radio_sendMsg("STANDBY");
				break;
			case RADIO_TX_ACTIVE:
				radio_sendMsg("TX_ACTIVE");
				break;
			case RADIO_RX_ACTIVE:
				radio_sendMsg("RX_ACTIVE");
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

	while (amount_read < amount_to_recieve) {

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

int radio_readOneByte() {
	uint8_t buffer[1];
	radio_readBytes(buffer, 1);
	return buffer[0];
//	return usart_recieveBufferNotEmpty(RADIO_USART);

}
