#pragma once
#include <stdlib.h>
#include "print_scan.h"
#include "suite.h"

UT_Suite* current_suite = NULL;
int current_test_failed = 0;
char* failure_message = NULL;


void init_suite(UT_Suite* suite, unsigned int id, char* name) {
    suite->id = id;
    suite->name = name;

    suite->num_tests = 0;
    suite->tests = NULL;

    suite->test_names = NULL;

    suite->passed = 0;
    suite->failed = 0;
}

void run_suite(UT_Suite* suite) {
    printMsg("\r\nSuite: %s\r\n\r\n", suite->name);
    // print_suite_details(suite);
    for (unsigned int i = 0; i < suite->num_tests; i++) {
        printMsg("  %s......\r\n", suite->test_names[i]);

        suite->tests[i]();

    
        if (current_test_failed) {
            printMsg("  Failed.\r\n\r\n");
            suite->failed++;
        } else {
            printMsg("  Passed.\r\n\r\n");
            suite->passed++;
        }

        // reset all extern variables

        current_test_failed = 0;
    }
}

void add_test(UT_Suite* suite, Test_Fun test_function, char* test_name) {
    Test_Fun* new_tests = realloc(suite->tests, (suite->num_tests + 1) * sizeof(Test_Fun));
    suite->tests = new_tests;
    suite->tests[suite->num_tests] = test_function;

    char** new_test_names = realloc(suite->test_names, (suite->num_tests + 1) * sizeof(test_name));
    suite->test_names = new_test_names;
    suite->test_names[suite->num_tests] = test_name;

    suite->num_tests++;
    printMsg("num tests %d", suite->num_tests);

}

void print_suite_details(UT_Suite* suite) {
    printMsg("\r\nSuite name: %s\r\n Suite ID: %d\r\n Suite Num Tests: %d\r\n", suite->name, suite->id, suite->num_tests);
}






