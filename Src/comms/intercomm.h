#include <globals.h>

#define ACK_TIMEOUT_MS	 10000	// 10 sec
#define READY_TIMEOUT_MS 20000	// 20 sec

/*
 * ACK_R  = x & 0xF0 = 0111
 * ACK_NR = h & 0xF0 = 0110
 * NACK   = H & 0xF0 = 0100
 * STOP   = X & 0xF0 = 0101
 * Tx     = . & 0xF0 = 0011
 * Rx     = . & 0xF0 = 0010
 *
 *
 */

// for identifying metadata
enum metadata_types {
	_Not_Metadata         = 0xF,
	_Heartbeat            = 0x0,
	_ACK_Ready            = 0x7,
	_ACK_NotReady         = 0x6,
	_NACK                 = 0x4,
	_STOP                 = 0x5,
	_Data_TransferRequest = 0x3,
	_Data_ReceiveRequest  = 0x2
};

// for identifying data types during a data transfer/reception
enum data_types {
	_Invalid             = 0xF,
	_ExperimentLogBuffer = 0x2,
	_ExperimentData      = 0x3,
	_EventLogBuffer      = 0x4,
	_EventsData          = 0x5,
	_TLE                 = 0x1
};

//-----------------------------------------------------------------------------

enum metadata_types intercomm_identifyMetadataType(char message);

enum data_types intercomm_identifyDataType(char message);

void intercomm_init();

bool intercomm_initiateTransfers(enum data_types type_of_data);

bool intercomm_TxRx(bool tx_or_rx, uint8_t data[], uint16_t size);

