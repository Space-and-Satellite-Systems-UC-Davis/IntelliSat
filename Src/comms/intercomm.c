#include "../system_config/UART/uart.h"
#include "intercomm.h"


#define IntercommUART 		LPUART1
#define InterCommBaudRate 	9600	// kbps

enum comm_wait_returns {
	__STOP,
	__TIMEDOUT,
	__NOTREADY,
	__READY
};

//-----------------------------------------------------------------------------

int8_t intercomm_waitForACK(uint16_t duration_ms) {
	uint64_t startMs = getSysTime();
	uint64_t currMs = 0;

	uint8_t recieved;
	while (true) {
		if (usart_recieveBytes(IntercommUART, &recieved, 1)) {
			if        (intercomm_identifyMetadataType(recieved) == _ACK_Ready)    {
				return __READY;
			} else if (intercomm_identifyMetadataType(recieved) == _ACK_NotReady) {
				return __NOTREADY;
			} else if (intercomm_identifyMetadataType(recieved) == _STOP)         {
				return __STOP;
			}
		}

		currMs = getSysTime();
		if (currMs - startMs >= duration_ms) {
			break;
		}
	}
	return __TIMEDOUT;
}

int8_t intercomm_waitForReady(uint16_t duration_ms) {
	uint64_t startMs = getSysTime();
	uint64_t currMs = 0;

	uint8_t recieved;
	while (true) {
		if (usart_recieveBytes(IntercommUART, &recieved, 1)) {
			if        (intercomm_identifyMetadataType(recieved) == _ACK_Ready)    {
				return __READY;
			} else if (intercomm_identifyMetadataType(recieved) == _STOP)         {
				return __STOP;
			}
		}

		currMs = getSysTime();
		if (currMs - startMs >= duration_ms) {
			break;
		}
	}
	return __TIMEDOUT;
}

void intercomm_stopCommunication() {
	uint8_t stopMessage = ((uint8_t)_STOP) << 4;
	usart_transmitBytes(IntercommUART, &stopMessage, 1);
}

//-----------------------------------------------------------------------------

enum metadata_types intercomm_identifyMetadataType(char message) {
	return (enum metadata_types)((message & 0xF0) >> 4);
}

enum data_types intercomm_identifyDataType(char message) {
	enum metadata_types type = intercomm_identifyMetadataType(message);
	if (type != _Data_TransferRequest || type != _Data_ReceiveRequest) {
		return _Invalid;
	}

	return (enum data_types)(message & 0x0F);
}

//-----------------------------------------------------------------------------

void intercomm_init() {
	usart_init(IntercommUART, InterCommBaudRate);
}

bool intercomm_initiateTransfers(enum data_types type_of_data) {
	uint8_t transferMessage = ((uint8_t)_Data_TransferRequest << 4) | (uint8_t)type_of_data;
	usart_transmitBytes(IntercommUART, &transferMessage, 1);

	int8_t commStatus = intercomm_waitForACK(ACK_TIMEOUT_MS);
	switch (commStatus) {
		case __TIMEDOUT:
			intercomm_stopCommunication();
			break;
		case __STOP:
			return false;
		case __READY:
			return true;
	}
	// case __NOTREADY: wait for Ready
	commStatus = intercomm_waitForReady(READY_TIMEOUT_MS);
	switch (commStatus) {
		case __TIMEDOUT:
			intercomm_stopCommunication();
			break;
		case __STOP:
			return false;
		case __READY:
			break;
	}

	return true;
}

bool intercomm_TxRx(bool tx_or_rx, uint8_t data[], uint16_t size) {
	uint8_t received;

	for (int i = 0; i < size; i++) {
		usart_transmitBytes(IntercommUART, &(data[i]), 1);

		if (usart_recieveBytes(IntercommUART, &received, 1)
				&& intercomm_identifyMetadataType(received) == _STOP) {
			return false;
		}
	}

	if (intercomm_waitForACK(ACK_TIMEOUT_MS) != __TIMEDOUT) {
		// ACK, or STOP, received
		return true;
	}

	return false;
}
