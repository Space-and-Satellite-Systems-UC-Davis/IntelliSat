#include "platform_init.h"
#include "UART/pcp.h"
#include "UART/uart.h"
#include "Buttons/buttons.h"

#include <TestDefinition.h>

#define RUN_TEST	0	// 0 = run IntelliSat, 1 = run a very specific test
#define TEST_ID 	0	// ID of the test to run in case RUN_TEST = 1

#define CHUNK_SIZE 8

int main() {
    init_init();
    init_platform(!RUN_TEST);
    buttons_init();

    usart_init(USART1, 9600);

    //Taken directly from Intercomm.h
    typedef enum {
        DownloadData = 'D', //Receive data from PFC
        UploadData = 'U', //Transfer data to PFC
    	SendState = 's', //Send state to PFC
    	TransferToGround = 'T', //Transfer data to ground station
    } MessageType;

    //Tells PFC what type of data is being sent
    //For future flags, try to keep them lowercase and don't step on other enums
    typedef enum {
        Unflagged = 'u', //No upload flags were designed yet
    } UploadFlag;

    typedef enum {
        TXactive = 't', //Busy tranceiving data to ground station
    	RXactive = 'r', //Busy receiving data from ground station
        Idle = 'o', //Not busy
    } State;

    PCPDevice pcp;
	make_pcpdev(&pcp, USART1);

	bool skip_one = false;


	//TEST D
	uint8_t chunk[CHUNK_SIZE] = {'D', 4};
	while(1) {
		bool button_status = get_buttonStatus_SW1();

		if (skip_one == false) {
			skip_one = true;
			continue; //status is true by default. Ignore once
		}

		if (button_status) {
			pcp_transmit(&pcp, chunk, CHUNK_SIZE);

			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < CHUNK_SIZE; j++) {
					chunk[j] = j+i + 48; //48 to get to ascii numbers
				}
				pcp_transmit(&pcp, chunk, CHUNK_SIZE);
			}

			break;
		}
	}

	while(1) {
		bool button_status = get_buttonStatus_SW1();
		if (button_status) {
			break;
		}
	}

	//TEST U
	while(1) {

		//Expect a 'U' request
		int read_status = pcp_read(&pcp, chunk);
		if (read_status == -1) {
			continue;
		}

		// ECHO
		usart_transmitBytes(USART1, chunk, CHUNK_SIZE);

		//Expect the chunks
		int n_chunks = chunk[1];

		for (int i = 0; i < n_chunks; i++) {
			pcp_read(&pcp, chunk);

			// ECHO
			usart_transmitBytes(USART1, chunk, CHUNK_SIZE);
			nop(10000);
		}

		break;
	}

	nop(100000);

	//TEST s
	uint8_t new_chunk[CHUNK_SIZE] = {'s'};
	while(1) {
		bool button_status = get_buttonStatus_SW1();

		if (button_status) {
			pcp_transmit(&pcp, new_chunk, CHUNK_SIZE);

			pcp_read(&pcp, new_chunk);

			// ECHO
			usart_transmitBytes(USART1, new_chunk, CHUNK_SIZE);

			break;
		}
	}

	//TEST T
	new_chunk[0] = 'T';
	new_chunk[1] = 4;
	while(1) {
		bool button_status = get_buttonStatus_SW1();

		if (button_status) {
			pcp_transmit(&pcp, new_chunk, CHUNK_SIZE);

			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < CHUNK_SIZE; j++) {
					new_chunk[j] = j+i + 48; //48 to get to ascii numbers
				}
				pcp_transmit(&pcp, new_chunk, CHUNK_SIZE);
			}

			break;
		}
	}
}
