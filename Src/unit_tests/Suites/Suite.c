#pragma once
#include <stdlib.h>
#include "print_scan.h"
#include "suite.h"

void init_suite(UT_Suite* suite, unsigned int id, char* name) {
    suite->id = id;
    suite->name = name;
    suite->num_tests = 0;
    suite->tests = NULL;
}

void run_suite(UT_Suite* suite) {
    // print_suite_details(suite);
    for (int i = 0; i < suite->num_tests; i++) {
        suite->tests[i]();
    }
}

void add_test(UT_Suite* suite, Test_Fun test_function) {
    Test_Fun* new_tests = realloc(suite->tests, (suite->num_tests + 1) * sizeof(Test_Fun));
    suite->tests = new_tests;
    suite->tests[suite->num_tests] = test_function;
    suite->num_tests++;
}

void print_suite_details(UT_Suite* suite) {
    printMsg("\r\nSuite name: %s\r\n Suite ID: %d\r\n Suite Num Tests: %d\r\n", suite->name, suite->id, suite->num_tests);
}

