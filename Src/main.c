#include "platform_init.h"
#include "unit_tests/unit_tests.h"

#define RUN_TEST	0	// 0 = run IntelliSat, 1 = run a very specific test
#define TEST_ID 	0	// ID of the test to run in case RUN_TEST = 1
#define RUN_UNIT_TESTS 0 //0 = run IntelliSat, 1 = run unit tests

#ifdef INC_FREERTOS_H

#include "FreeRTOS.h"
#include "task.h"

#define RUN_TEST_MAIN  0 // 0 to run scheduler main, 1 to run schduler test main

#endif

#include <TestDefinition.h>

int main() {
    init_init();
    init_platform(!RUN_TEST);
#if (RUN_TEST==1) && (TEST_ID != 0)
    void (*testFunc)();
    testFunc = getTestFunction(TEST_ID);
    testFunc();

#elif (RUN_TEST == 0) && (RUN_UNIT_TESTS == 1)
	run_tests();
  
#else
    #ifdef INC_FREERTOS_H
        #if (RUN_TEST_MAIN == 0)
            scheduler_main();
        #else
            scheduler_test_main();
        #endif
    #endif
    
	while (1) {}
#endif

}