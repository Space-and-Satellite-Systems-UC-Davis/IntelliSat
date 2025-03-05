#include <print_scan.h>
#include <TestDefinition.h>
#include "platform_init.h"
#include "UART/uart.h"

#define RUN_TEST	0	// 0 = run IntelliSat, 1 = run a very specific test
const Test test = TEST_NOTHING;

#include <TestDefinition.h>

int main() {
    init_init();
    init_platform(!RUN_TEST);
    // ^ don't want to run the Scheduler in case we are running other tests

#if (RUN_TEST==1)

    void (*testFunc)();
    testFunc = getTestFunction(test);
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
