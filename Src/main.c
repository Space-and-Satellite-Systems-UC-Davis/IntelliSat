#include <print_scan.h>
#include <TestDefinition.h>
#include "platform_init.h"
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

	//TODO: use RTC first_time flag.
	//if (first_time) {
	//  init_first_time()
	//}

//    uint8_t buffer[1] = {111};
	while (1) {
		printMsg("THIS WORKSSSSSS\n\r");
		nop(1000);
		printMsg("STILL DOES\n\r");
//		int num_bytes = usart_receiveBytes(USART1, buffer, 1);
//		continue;
	}

#endif

}
