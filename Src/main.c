#include <print_scan.h>
#include <TestDefinition.h>
#include "platform_init.h"
#include "UART/pcp.h"
#include "UART/uart.h"
#include "Buttons/buttons.h"

#define RUN_TEST	0	// 0 = run IntelliSat, 1 = run a very specific test
#define TEST_ID 1 // ID of the test to run in case RUN_TEST = 1

#define CHUNK_SIZE 8

#include <TestDefinition.h>

int main() {
    init_init();
    init_platform(!RUN_TEST);
    buttons_init();


#if (RUN_TEST==1)

    void (*testFunc)();
    testFunc = getTestFunction(TEST_ID);
    testFunc();

    #else

    usart_init(USART1, 9600);

    PCPDevice pcp;
	make_pcpdev(&pcp, USART1);

	uint8_t chunk[CHUNK_SIZE];

	const int CHUNK_LENGTH = 8;

	while(1) {
		int read_status = pcp_read(&pcp, chunk);
		if (read_status != -1) {
			uint8_t n_chunks = chunk[1];

			for (int i = 0; i < n_chunks; i++) {
				while(1) {
					int read_status = pcp_read(&pcp, chunk);
					if (read_status != -1) {
						//ECHO
			    		for (int j = 0; j < CHUNK_LENGTH; j++) {
				    		usart_transmitBytes(USART1, chunk, CHUNK_LENGTH);
			    		}

						//HERE BE DRAGONS
						//Store the data here

						break;
					}
				}
			}
		}
	}



//    uint8_t chunk[CHUNK_SIZE] = {'T', 4};
//
//    bool skip_one = false;
//
//    while(1) {
//        bool button_status = get_buttonStatus_SW1();
//
//        if (skip_one == false) {
//        	skip_one = true;
//        	continue; //status is true by default. Ignore once
//        }
//
//        if (button_status) {
//            pcp_transmit(&pcp, chunk, CHUNK_SIZE);
//
//            for (int i = 0; i < 4; i++) {
//                nop(100000);
//            	for (int j = 0; j < CHUNK_SIZE; j++) {
//            		chunk[j] = j+i + 48; //48 to get to ascii numbers
//            	}
//                pcp_transmit(&pcp, chunk, CHUNK_SIZE);
//            }
//        }
//    }

//	//Make it so it does the transmit and loop only if button is pressed ?
//	while (1) {
//		pcp_retransmit(&pcp);
//
////		usart_transmitChar(USART1, 'C');
////		usart_transmitChar(USART1, 'C');
////		usart_transmitChar(USART1, 'C');
//
//		nop(100000);
//	}

#endif

}
