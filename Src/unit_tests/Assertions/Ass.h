#pragma once
#include "print_scan.h"
#include "globals.h"

// equality
bool assert_equal(int actual, int expected);
bool assert_equal_int(int actual, int expected);
bool assert_equal_float_exact(float actual, float expected);
bool assert_equal_float_within(float actual, float expected, float leeway);

// char array
bool assert_equal_char_array(char* actual, char* expected, int length); // or size_t length

// integers
bool assert_greater_than_int(int greater, int lesser);
bool assert_greater_than_equal_int(int greater, int lesser);
bool assert_less_than_int(int lesser, int greater);
bool assert_less_than_equal_int(int lesser, int greater);

// floats 
bool assert_greater_than_float(float greater, float lesser, float leeway);
bool assert_greater_than_equal_float(float greater, float lesser, float leeway);
bool assert_less_than_float(float lesser, float greater, float leeway);
bool assert_less_than_equal_float(float lesser, float greater, float leeway);
