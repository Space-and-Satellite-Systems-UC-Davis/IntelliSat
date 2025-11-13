#include <print_scan.h>
#include "platform_init.h"
#include "unit_tests/unit_tests.h"

#define RUN_TEST	0	// 0 = run IntelliSat, 1 = run a very specific test
#define TEST_ID 	17	// ID of the test to run in case RUN_TEST = 1
#define RUN_UNIT_TESTS 1 //0 = run IntelliSat, 1 = run unit tests

#include <TestDefinition.h>

int main() {
    init_init();
    init_platform(!RUN_TEST);
	printMsg("hello world");

#if (RUN_TEST==1) && (TEST_ID != 0)

    void (*testFunc)();
    testFunc = getTestFunction(TEST_ID);
    testFunc();

#elif (RUN_TEST == 0) && (RUN_UNIT_TESTS == 1)
	printMsg("goodbye world");
	run_tests();

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
