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

void init_suite(UT_Suite* suite, unsigned int id, char* name);
void run_suite(UT_Suite* suite);
void add_test(UT_Suite* suite, Test_Fun test_function, char* test_name);

// lets the assert functions access these 
extern UT_Suite* current_suite;
extern int current_test_failed;

#endif