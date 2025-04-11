#include <print_scan.h>
#include "platform_init.h"

#define RUN_TEST	0	// 0 = run IntelliSat, 1 = run a very specific test
#define TEST_ID 	10	// ID of the test to run in case RUN_TEST = 1

#include <TestDefinition.h>
#include "../ADCS-Software/ADCS.h"

int main() {
    init_init();
    init_platform(!RUN_TEST);

#if (RUN_TEST==1) && (TEST_ID != 0)

    void (*testFunc)();
    testFunc = getTestFunction(TEST_ID);
    testFunc();

    #else

	//TODO: use RTC first_time flag.
	//if (first_time) {
	//  init_first_time()
	//}

    ADCS_MAIN(ADCS_HDD_EXP_ANGVEL);

	while (1) {
		continue;
	}

#endif

}
