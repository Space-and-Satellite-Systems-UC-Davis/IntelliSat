#include "./unity.h"

void test_basic_example_unity();

// NOTE: These are UNIT tests not related to ADCS
int run_unit_tests() {
    UNITY_BEGIN();

	// NOTE: Add all tests with the RUN_TEST function
    RUN_TEST(test_basic_example_unity);

    return UNITY_END();
}

void test_basic_example_unity() {
    TEST_ASSERT(1);

    // Try to use type specific functions as much as possible, as they produce
    // better debugging output!
    TEST_ASSERT_EQUAL_INT(1, 1);
}
