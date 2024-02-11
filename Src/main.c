#include "platform_init.h"
#include "tools/print_scan.h"

/*
 * For now this won't be implemented on the main branch
 * Therefore, the main branch cannot be built.
 * Each development / feature branch has it's own implementation
 * which is used for testing specific features.
 */
void branch_main();

int main() {
	init_platform();
	branch_main();
}






//-----------------------------------------------------------------------------

/*
#define IntercommUART UART1

void intercomm_init() {
	usart_init(IntercommUART, 9600);
}

enum metadata_types {
	_Not_Metadata,
	_Heartbeat,
	_ACK_Ready,
	_ACK_NotReady,
	_NACK,
	_STOP,
	_Data_TransferRequest,
	_Data_ReceiveRequest
};

enum data_types {
	_Invalid,
	_ExperimentLogBuffer,
	_ExperimentData,
	_EventLogBuffer,
	_EventsData,
	_TLE
};

enum metadata_types intercomm_identifyMetadataType(char message) {
	// parse and return right value: top 4 bits?
}

enum data_types intercomm_identifyDataType(char message) {
	// if not a data request, return _Invalid
	enum metadata_types type = intercomm_identifyMetadataType(message);
	if (type != _Data_TransferRequest && type != _Data_ReceiveRequest_) {
		return _Invalid;
	}

	// parse and return the right value: bottom 4 bits?
}

// -2 : Timedout, -1 : STOP, 0 : Not Ready, 1 : Ready
uin8_t intercomm_waitForACK() {
	uint64_t start_ms = getSysTime(), curr_ms = 0;
	char recieved;
	while (true) {
		if (usart_recieveBytes(IntercommUART, &recieved, 1)) {
			if        (intercomm_identifyMetadataType(recieved) == _ACK_Ready)    {
				return 1;
			} else if (intercomm_identifyMetadataType(recieved) == _ACK_NotReady) {
				return 0;
			} else if (intercomm_identifyMetadataType(recieved) == _STOP)         {
				return -1;
			}
		}
		curr_ms = getSysTime();
		if (curr_ms - start_ms >= 10) {
			break;
		}
	}
	return -2;
}

void intercomm_stopCommunication() {
	// construct STOP message
	usart_transmitBytes(IntercommUART, &stop_message, 1);
}

bool intercomm_initiateTransfers() {
	// send Transfer Request

	uin8_t ack_status = intercomm_waitForACK();


	// If ACK is not Ready, wait for Ready


	return true;
}

bool intercomm_TxRx(bool tx_or_rx, uint8_t data[], uint16_t size) {
	char recievied;

	for (int i = 0; i < size; i++) {
		usart_transmitBytes(IntercommUART, data[i], 1);

		if (usart_recieveBytes(IntercommUART, &recieved, 1)
				&& intercomm_identifyMetadataType(recieved) == _STOP) {
			return false;
		}

	}

	if (intercomm_waitForACK() != -2) {		// ACK, or STOP, received
		return true;
	}

	return false;
}


bool comms_DataSharingFunctionTransmit(enum data_types type_of_data) {
	// Get the Data Based on the type_of_data
	uint8_t *data;
	uint16_t size;

	if (intercomm_initiateTransfers() == false) {
		intercomm_stopCommunication();
		return false;
	}

	bool success = intercomm_TxRx(true, data, size);

	return success;
}
*/
