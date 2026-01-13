#include "globals.h"
#include "IMU_Tests/IMU_Tests.h"
#include "Suites/suite.h"
#include "print_scan.h"
#include "Assertions/Ass.h"

void test_function1()
{
    assert_equal_int(1, 1);
    assert_equal_int(1, 2);
    assert_equal_int(2,4);
}

void test_function2()
{
    assert_equal_int(45,45);
    assert_equal_float_exact(0.25f, 0.251f);
}

void test_function3()
{
    assert_equal_int(3, 3);
}


void run_tests()
{
    // IMU Tests
    //  test_initial_state();
    UT_Suite suite;
    init_suite(&suite, 1, "First suite");
    add_test(&suite, test_function1, "first test");
    add_test(&suite, test_function2, "second test");
    // add_test(&suite, test_function2);
    add_test(&suite, test_function3, "third test");
    run_suite(&suite);
}
