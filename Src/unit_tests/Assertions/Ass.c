#include "Ass.h"

#include <string.h>
#include <math.h>
#include <stdio.h>
#include <LED/led.h>

bool assert_equal(int actual, int expected) {
    return actual == expected;
}
 
// equal 
bool assert_equal_int(int actual, int expected) {
    return actual == expected;
}

bool assert_equal_float_exact(float actual, float expected) {
    uint32_t actual_bits;
    uint32_t expected_bits;

    memcpy(&actual_bits, &actual, sizeof(float));
    memcpy(&expected_bits, &expected, sizeof(float));

    return actual_bits == expected_bits;
}

bool assert_equal_float_within(float actual, float expected, float leeway) {
    // allow leeway to prevent floating point precision errors
    return fabsf(actual - expected) <= leeway;
}

// char array
bool assert_equal_char_array(char* actual, char* expected, int length) {
    for (int i = 0; i < length; i++) {
        if (actual[i] != expected[i]) {
            return false;
        }
    }

    return true;
}

// greater than int
bool assert_greater_than_int(int greater, int lesser) {
    return greater > lesser;
}

bool assert_greater_than_equal_int(int greater, int lesser) {
    return greater >= lesser;
}

// greater than float
bool assert_greater_than_float(float greater, float lesser, float leeway) {
    return greater - fabsf(leeway) > lesser;
}

bool assert_greater_than_equal_float(float greater, float lesser, float leeway) {
    return greater - fabsf(leeway) > lesser || assert_equal_float_exact(greater, lesser);
}

// less than int
bool assert_less_than_int(int lesser, int greater) {
    return lesser < greater;
}

bool assert_less_than_equal_int(int lesser, int greater) {
    return lesser <= greater;
}

// less than float
bool assert_less_than_float(float lesser, float greater, float leeway) {
    return lesser < greater - fabsf(leeway);
}

bool assert_less_than_equal_float(float lesser, float greater, float leeway) {
    return lesser < greater - fabsf(leeway) || assert_equal_float_exact(lesser, greater);
}