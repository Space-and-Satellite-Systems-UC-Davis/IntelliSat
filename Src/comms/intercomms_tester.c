#include "intercomm.h"
#include "../system_config/UART/uart.h"
#include "../system_config/Buttons/buttons.h"
#include "../tools/print_scan.h"


void intercomm_tester1() {
	/** Test Description
	 *
	 * Sending and receiving characters
	 * to see if they are correctly decoded
	 */

	printMsg("Stating Test: Parsing\r\n");

	uint8_t rec = NULL;
	while (1) {
		usart_recieveBytes(USART1, &rec, 1);

		enum metadata_types type = intercomm_identifyMetadataType(rec);
		switch(type) {
			case _Not_Metadata:
				printMsg("Nope\r\n");
				break;
			case _ACK_Ready:
				printMsg("ACK: READY\r\n");
				break;
			case _ACK_NotReady:
				printMsg("ACK: WAIT\r\n");
				break;
			case _NACK:
				printMsg("NACK\r\n");
				break;
			case _STOP:
				printMsg("STOP\r\n");
				break;
			case _Data_TransferRequest:
				printMsg("Transfer\r\n");
				break;
			case _Data_ReceiveRequest:
				printMsg("Receive\r\n");
				break;
		}

		enum data_types type2 = intercomm_identifyDataType(rec);
		switch(type) {
			case _ExperimentLogBuffer:
				printMsg("Experiment Logs\r\n");
				break;
			case _ExperimentData:
				printMsg("All Experiments\r\n");
				break;
			case _EventLogBuffer:
				printMsg("Event Logs\r\n");
				break;
			case _EventsData:
				printMsg("All Events\r\n");
				break;
			case _TLE:
				printMsg("TLE\r\n");
				break;
		}

		if (rec == '`') {
			break;
		} else {
			rec = NULL;
		}


	}


}

extern int8_t intercomm_waitForACK(uint16_t duration_ms);
extern int8_t intercomm_waitForReady(uint16_t duration_ms);

void intercomm_tester2() {
	/** Test Description
	 *
	 * Checking if the Timeout Functions work correctly
	 * ACK Timeout: 10 sec
	 * Ready Timeout: 20 sec
	 */
}

void intercomm_tester3() {
	/** Test Description
	 *
	 * Checking if the Transfer initiates correctly
	 */
	printMsg("Starting Test: TX Initiate\r\n");

	bool success = intercomm_initiateTransfers(_ExperimentLogBuffer);

	if (success) {
		printMsg("Successful!\r\n");
	} else {
		printMsg("Unsuccessful :(\r\n");
	}
}



void branch_main() {
	// intercomm_init();

	while (1) {
		if (get_buttonStatus_0()) {
			intercomm_tester1();
		}
	}
}
