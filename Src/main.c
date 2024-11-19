#include <print_scan.h>
#include "platform_init.h"

#define RUN_TEST	1	// 0 = run IntelliSat, 1 = run a very specific test
#define TEST_ID 	5	// ID of the test to run in case RUN_TEST = 1

#include <TestDefinition.h>

int main() {
    init_init();
    init_platform(!RUN_TEST);
    // ^ don't want to run the Scheduler in case we are running other tests

#if (RUN_TEST==1) && (TEST_ID != 0)

    void (*testFunc)();
    testFunc = getTestFunction(TEST_ID);
    testFunc();

    #else

	//TODO: use RTC first_time flag.
	//if (first_time) {
	//  init_first_time()
	//}

	while (1) {
		continue;
	}

#endif

}
