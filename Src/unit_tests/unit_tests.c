#include "globals.h"
#include "IMU_Tests/IMU_Tests.h"
#include "Suites/suite.h"
#include "print_scan.h"

void test_function1()
{
    printMsg("\r\nRun test function 1\r\n");
}

void test_function2()
{
    printMsg("\r\nRun test function 2\r\n");
}


void run_tests()
{
    // IMU Tests
    //  test_initial_state();
    UT_Suite suite;
    init_suite(&suite, 1, "First suite");
    add_test(&suite, test_function1);
    add_test(&suite, test_function2);
    run_suite(&suite);
}
