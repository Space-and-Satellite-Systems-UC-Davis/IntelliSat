#include <print_scan.h>
#include <TestDefinition.h>
#include <UART/uart.h>
#include "platform_init.h"

#define RUN_TEST	0	// 0 = run IntelliSat, 1 = run a very specific test
const Test test = TEST_NOTHING;

#include <TestDefinition.h>

int main() {
    init_init();
    init_platform(!RUN_TEST);

#if (RUN_TEST==1)

    void (*testFunc)();
    testFunc = getTestFunction(test);
    testFunc();

    #else

	//TODO: use RTC first_time flag.
	//if (first_time) {
	//  init_first_time()
	//}

	usart_init(USART2, 9600);
	uint8_t buffer[1] = {111};

	while (1) {
		printMsg("TICK\n\r");
        usart_receiveBytes(USART2, buffer, 1);
        printMsg("Buffer value: %d \n\r", buffer[0]);
        nop(1000);
		continue;
	}

#endif

}
