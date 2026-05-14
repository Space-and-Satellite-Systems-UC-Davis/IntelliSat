#pragma once
#include "print_scan.h"
#include "globals.h"

// use assert_equal_int instead
bool assert_equal(int actual, int expected);

// integers
/**
 * @brief Checks for integer equality.
 * 
 * Checks the exact bit values. Doesn't print any message.
 * 
 * @param actual The variable to check
 * @param expected The value the variable should be.
 * @return Boolean.
 */
bool assert_equal_int(int actual, int expected);
/**
 * @brief Checks integer greater than.
 * 
 * @param greater The expected larger variable
 * @param lesser The expected smaller variable
 * @return Boolean.
 */
bool assert_greater_than_int(int greater, int lesser);
/**
 * @brief Checks integer greater than equal to 
 * 
 * @param greater The expected larger variable
 * @param lesser The expected smaller variable
 * @return Boolean.
 */
bool assert_greater_than_equal_int(int greater, int lesser);
/**
 * @brief Checks integer less than
 * 
 * @param lesser The expected smaller variable
 * @param greater The expected larger variable
 * @return Boolean.
 */
bool assert_less_than_int(int lesser, int greater);
/**
 * @brief Checks integer less than equal to
 * 
 * @param lesser The expected smaller variable
 * @param greater The expected larger variable
 * @return Boolean.
 */
bool assert_less_than_equal_int(int lesser, int greater);

// floats 
/**
 * @brief Checks for integer equality.
 * 
 * Checks the exact bit values.
 * 
 * @param actual The variable to check
 * @param expected The value the variable should be.
 * @return Boolean.
 */
bool assert_equal_float_exact(float actual, float expected);
/**
 * @brief Checks for float equality within a certain margin of error.
 * 
 * If a float does not have to equal a value exactly.
 * 
 * @param actual The variable to check
 * @param expected The value the variable should be.
 * @param leeway The largest difference between actual and expected float value
 * @return Boolean.
 */
bool assert_equal_float_within(float actual, float expected, float leeway);
/**
 * @brief Checks float greater than.
 * 
 * @param greater The expected larger variable
 * @param lesser The expected smaller variable
 * @param leeway The allowed margin of error between the comparison (i.e. 0.3 with leeway 0.0001 makes 0.3 > 0.3 true)
 * @return Boolean.
 */
bool assert_greater_than_float(float greater, float lesser, float leeway);
/**
 * @brief Checks float greater than or equal.
 * 
 * @param greater The expected larger variable
 * @param lesser The expected smaller variable
 * @param leeway The allowed margin of error between the comparison (i.e. 0.3 with leeway 0.1 makes 0.3 >= 0.31 true)
 * @return Boolean.
 */
bool assert_greater_than_equal_float(float greater, float lesser, float leeway);
/**
 * @brief Checks float less than
 * 
 * @param lesser The expected smaller variable
 * @param greater The expected larger variable
 * @param leeway The allowed margin of error in comparison (i.e. leeway of 0.1 makes 0.3 < 0.3 true)
 * @return Boolean.
 */
bool assert_less_than_float(float lesser, float greater, float leeway);
/**
 * @brief Checks float less than or equal to
 * 
 * @param lesser The expected smaller variable
 * @param greater The expected larger variable
 * @param leeway The allowed margin of error in comparison (i.e. leeway of 0.1 makes 0.3 <= 0.2 true)
 * @return Boolean.
 */
bool assert_less_than_equal_float(float lesser, float greater, float leeway);

// char array
/**
 * @brief String equality
 * 
 * Checks if all characters in a string match.
 * 
 * @param actual Char array of the string variable
 * @param expected Char array of what the string should be
 * @param length Length of the character strings. Should be same for both actual and expected
 * @return Boolean.
 */
bool assert_equal_char_array(char* actual, char* expected, int length); // or size_t length

// over time
bool assert_stays_within_over_time_int();