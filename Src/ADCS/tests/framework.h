#pragma once
#include <math.h>
#include <stdio.h>

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

#define TEST_ASSERT_DBL(a, b, tol)                                             \
    TEST_ASSERT(fabs((double)(a) - (double)(b)) <= (double)(tol))

#define TEST_ASSERT_FLT(a, b, tol)                                             \
    TEST_ASSERT(fabsf((float)(a) - (float)(b)) <= (float)(tol))

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
