#pragma once
#include <stdio.h>

/*
 * Host-side test macro harness for the Startup FRAM provisioning tools, matching
 * the pattern used by Src/startup/tests/framework.h (which itself was copied
 * from Src/scheduler/tests/framework.h). Copied rather than shared so this lane
 * owns its own files and cannot break the firmware lane's suite.
 */

static int _tests_run = 0;
static int _tests_failed = 0;

#define TEST_SUITE(name) printf("\n=== %s ===\n", (name))

#define TEST_ASSERT(cond)                                                      \
    do {                                                                       \
        _tests_run++;                                                          \
        if (!(cond)) {                                                         \
            printf("  FAIL [%s:%d]: %s\n", __FILE__, __LINE__, #cond);         \
            _tests_failed++;                                                   \
        }                                                                      \
    } while (0)

#define RUN_TEST(fn)                                                           \
    do {                                                                       \
        printf("  %s\n", #fn);                                                 \
        fn();                                                                  \
    } while (0)

#define TEST_RESULTS()                                                         \
    do {                                                                       \
        int _passed = _tests_run - _tests_failed;                              \
        printf("\n%d/%d passed", _passed, _tests_run);                         \
        if (_tests_failed)                                                     \
            printf(" (%d FAILED)", _tests_failed);                             \
        printf("\n");                                                          \
        return _tests_failed ? 1 : 0;                                          \
    } while (0)
