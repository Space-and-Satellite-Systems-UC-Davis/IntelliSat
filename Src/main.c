#include <print_scan.h>
#include "platform_init.h"

#define RUN_TEST	1	// 0 = run IntelliSat, 1 = run a very specific test
#define TEST_ID 	4	// ID of the test to run in case RUN_TEST = 1

// NOTE: These are ADCS scientific tests and NOT unit tests
#include <TestDefinition.h>

// NOTE: These are UNIT tests not related to ADCS
int run_unit_tests() {
	UNITY_BEGIN();

	runAllTests();

	return UNITY_END();
}

int main() {
    init_init();
    init_platform(!RUN_TEST);
    // ^ don't want to run the Scheduler in case we are running other tests

#if (RUN_TEST==1)

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
