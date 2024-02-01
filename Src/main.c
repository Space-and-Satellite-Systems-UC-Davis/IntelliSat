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






//


// TODO: Functions to check if char is a certain MetaDataType

enum data_types {
	_ExperimentLogBuffer,
	_ExperimentData,
	_EventLogBuffer,
	_EventsData,
	_TLE
	// TOOD: ???
};

void intercomm_stopCommunication() {
	// TODO
}

bool intercomm_initiateTransfers() {
	// send Transfer

	// wait for ACK
	uint64_t start_ms = getSysTime(), curr_ms = 0;
	bool ready = false;

	// If ACK is not Ready, wait for Ready

	char message;
	while (!ready) {
		// check if a message was received, and if it's an ack with ready

		// if message is STOP, return false?

		curr_ms = getSysTime();
		if (curr_ms - start_ms >= 10) {
			intercomm_stopCommunication();
			return false;
		}

	}


}

bool intercomm_TxRx(bool tx_or_rx, uint8_t data[], uint16_t size) {
	// Send Data

	// Confirm that ACK was recieved

	return false;
}


bool comms_DataSharingFunctionTransmit(enum data_types type_of_data) {
	// Get the Data Based on the type_of_data
	uint8_t *data;
	uint16_t size;

	// Initiate the InterComm Requests
	if (intercomm_initiateTransfers() == false) {
		intercomm_stopCommunication();
		return false;
	}

	bool success = intercomm_TxRx(true, data, size);

	return success;
}
