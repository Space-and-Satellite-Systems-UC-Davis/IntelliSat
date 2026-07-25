#ifndef SUITE_H
#define SUITE_H

typedef void (*Test_Fun)();

typedef struct UT_Suite {
    unsigned int id;
    char* name;

    // tests
    unsigned int num_tests;
    Test_Fun* tests;
    char** test_names;

    int passed;
    int failed;
} UT_Suite;

/**
 * @brief Initializes values for a test suite. Must first intialize a UT_Suite variable
 * 
 * @param suite pointer to a UT_Suite variable
 * @param id The suite id
 * @param name A char array containing a name for the test suite
 * @return None.
 */
void init_suite(UT_Suite* suite, unsigned int id, char* name);
/**
 * @brief Runs a suite and all the tests inside of it.
 * 
 * @param suite The suite to run
 * @return None
 */
void run_suite(UT_Suite* suite);
/**
 * @brief Add a test function to a suite
 * 
 * @param suite Pointer to the suite to add the test to
 * @param test_function void function() containing asserts to be run
 * @param test_name char array for the test's display name
 * @return None.
 */
void add_test(UT_Suite* suite, Test_Fun test_function, char* test_name);
/**
 * @brief Print basic details about the suite (name, id, num tests)
 * 
 * @param suite The suite to print details for
 * @return None.
 */
void print_suite_details(UT_Suite* suite);

// lets the assert functions access these 
extern UT_Suite* current_suite;
extern int current_test_failed;

#endif