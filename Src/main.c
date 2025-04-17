#include <print_scan.h>
#include <TestDefinition.h>
#include "platform_init.h"
#include "UART/pcp.h"
#include "UART/uart.h"

#define RUN_TEST	0	// 0 = run IntelliSat, 1 = run a very specific test
#define TEST_ID 1 // ID of the test to run in case RUN_TEST = 1

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


	//TODO: use RTC first_time flag.
	//if (first_time) {
	//  init_first_time()
	//}
    PCPDevice pcp;
	make_pcpdev(&pcp, USART1);

	pcp_transmit(&pcp , "H", 2);

    uint8_t buffer[2] = {13, 222};
	while (1) {
		//pcp_read(&pcp, buffer);
//		printMsg("Buffer value: %d \n\r", buffer[0]);
		nop(1000000);

		pcp_retransmit(&pcp);
	}


#endif

}
