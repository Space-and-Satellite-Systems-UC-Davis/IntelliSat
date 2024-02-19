#include "../globals.h"

#define ACK_TIMEOUT_MS	 10
#define READY_TIMEOUT_MS 20

// for identifying metadata
enum metadata_types {
	_Not_Metadata = 0xF,
	_Heartbeat = 0x0,
	_ACK_Ready = 0x2,
	_ACK_NotReady = 0x1,
	_NACK = 0x3,
	_STOP = 0x4,
	_Data_TransferRequest = 0x5,
	_Data_ReceiveRequest = 0x6
};

// for identifying data types during a data transfer/reception
enum data_types {
	_Invalid = 0xF,
	_ExperimentLogBuffer = 0x2,
	_ExperimentData = 0x3,
	_EventLogBuffer = 0x4,
	_EventsData = 0x5,
	_TLE = 0x1
};

//-----------------------------------------------------------------------------

enum metadata_types intercomm_identifyMetadataType(char message);

enum data_types intercomm_identifyDataType(char message);

void intercomm_init();

bool intercomm_initiateTransfers(enum data_types type_of_data);

bool intercomm_TxRx(bool tx_or_rx, uint8_t data[], uint16_t size);

