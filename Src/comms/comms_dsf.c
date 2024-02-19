#include "intercomm.h"

#define BUFFERLEN 1024

uint8_t Buffer[BUFFERLEN];

bool comms_DataSharingFunctionTransmit(enum data_types type_of_data) {
	// TODO: Get the Data Based on the type_of_data
	uint8_t *data = Buffer;
	uint16_t size = BUFFERLEN;

	if (intercomm_initiateTransfers(type_of_data) == false) {
		return false;
	}

	bool success = intercomm_TxRx(true, data, size);

	return success;
}

//------------------------------------------------------------------------

void branch_main() {


	while (1) {
		// ...
	}
}
