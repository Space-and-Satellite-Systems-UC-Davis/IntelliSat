#include <print_scan.h>
#include <TestDefinition.h>
#include "platform_init.h"
#include "UART/pcp.h"
#include "UART/uart.h"

#define RUN_TEST	0	// 0 = run IntelliSat, 1 = run a very specific test
#define TEST_ID 1 // ID of the test to run in case RUN_TEST = 1

#define CHUNK_SIZE 8

#include <TestDefinition.h>

int main() {
    init_init();
    init_platform(!RUN_TEST);


#if (RUN_TEST==1)

    void (*testFunc)();
    testFunc = getTestFunction(TEST_ID);
    testFunc();

    #else

    usart_init(USART1, 9600);

    PCPDevice pcp;
	make_pcpdev(&pcp, USART1);

    uint8_t chunk[CHUNK_SIZE] = {'s'};

    pcp_transmit(&pcp, chunk, CHUNK_SIZE);

    //Make it so it does the transmit and loop only if button is pressed ?
	while (1) {
		pcp_retransmit(&pcp);

//		usart_transmitChar(USART1, 'C');
//		usart_transmitChar(USART1, 'C');
//		usart_transmitChar(USART1, 'C');

		nop(10000000);
	}


#endif

}
