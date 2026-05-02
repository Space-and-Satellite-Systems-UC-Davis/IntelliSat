#include "Ass.h"
#include "../Suites/suite.h"

#include <string.h>
#include <math.h>
#include <stdio.h>
#include <LED/led.h>
#include <inttypes.h>

/**
 * @brief Checks for float exact equality. 
 * 
 * Checks the exact bit values. Doesn't print any message.
 * 
 * @param actual The variable to check
 * @param expected The value the variable should be.
 * @return Boolean.
 */
bool assert_equal_float_exact_no_print(float actual, float expected)
{
    uint32_t actual_bits;
    uint32_t expected_bits;

    memcpy(&actual_bits, &actual, sizeof(float));
    memcpy(&expected_bits, &expected, sizeof(float));

    if (actual_bits != expected_bits)
    {
        return false;
    }

    return true;
}

// don't
bool assert_equal(int actual, int expected)
{
    return actual == expected;
}

/**
 * @brief Checks for integer equality.
 * 
 * Checks the exact bit values. Doesn't print any message.
 * 
 * @param actual The variable to check
 * @param expected The value the variable should be.
 * @return Boolean.
 */
bool assert_equal_int(int actual, int expected)
{
    if (actual != expected)
    {
        current_test_failed = 1;
        printMsg("\tExpected: %d, Received: %d\r\n", expected, actual);
        return false;
    }

    return true;
}

/**
 * @brief Checks for integer equality.
 * 
 * Checks the exact bit values.
 * 
 * @param actual The variable to check
 * @param expected The value the variable should be.
 * @return Boolean.
 */
bool assert_equal_float_exact(float actual, float expected)
{
    uint32_t actual_bits;
    uint32_t expected_bits;

    memcpy(&actual_bits, &actual, sizeof(float));
    memcpy(&expected_bits, &expected, sizeof(float));

    if (actual_bits != expected_bits)
    {
        current_test_failed = 1;
        printMsg("\tExpected: %f (bits %" PRIu32 "), Received: %f (bits %" PRIu32 ")\r\n", expected, expected_bits, actual, actual_bits);
        return false;
    }

    return true;
}

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
bool assert_equal_float_within(float actual, float expected, float leeway)
{
    // allow leeway to prevent floating point precision errors

    if (!(fabsf(actual - expected) <= leeway))
    {
        current_test_failed = 1;
        printMsg("\tExpected: %f with leeway of %f, Received: %f\r\n", expected, leeway, actual);
        return false;
    }

    return true;
}

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
bool assert_equal_char_array(char *actual, char *expected, int length)
{
    for (int i = 0; i < length; i++)
    {
        if (actual[i] != expected[i])
        {
            current_test_failed = 1;
            printMsg("Expected: \'");
            for (int j = 0; j < length; j++)
            {
                printMsg("%c", actual[j]);
            }
            printMsg("\', Received: \'");
            for (int j = 0; j < length; j++)
            {
                printMsg("%c", expected[j]);
            }
            printMsg("\'.\r\n");
            return false;
        }
    }

    return true;
}

/**
 * @brief Checks integer greater than.
 * 
 * @param greater The expected larger variable
 * @param lesser The expected smaller variable
 * @return Boolean.
 */
bool assert_greater_than_int(int greater, int lesser)
{
    if (greater > lesser)
    {
        return true;
    }

    current_test_failed = 1;
    printMsg("\tExpected %d to be greater than %d\r\n", greater, lesser);
    return false;
}

/**
 * @brief Checks integer greater than equal to 
 * 
 * @param greater The expected larger variable
 * @param lesser The expected smaller variable
 * @return Boolean.
 */
bool assert_greater_than_equal_int(int greater, int lesser)
{
    if (greater >= lesser)
    {
        return true;
    }

    current_test_failed = 1;
    printMsg("\tExpected %d to be greater than or equal to %d\r\n", greater, lesser);
    return false;
}

/**
 * @brief Checks float greater than.
 * 
 * @param greater The expected larger variable
 * @param lesser The expected smaller variable
 * @param leeway The allowed margin of error between the comparison (i.e. 0.3 with leeway 0.0001 makes 0.3 > 0.3 true)
 * @return Boolean.
 */
bool assert_greater_than_float(float greater, float lesser, float leeway)
{
    if (greater - fabsf(leeway) > lesser || greater + fabsf(leeway) > lesser )
    {
        return true;
    }

    current_test_failed = 1;
    printMsg("\tExpected %f to be greater than %f with leeway %f\r\n", greater, lesser, leeway);
    return false;
}

/**
 * @brief Checks float greater than or equal.
 * 
 * @param greater The expected larger variable
 * @param lesser The expected smaller variable
 * @param leeway The allowed margin of error between the comparison (i.e. 0.3 with leeway 0.1 makes 0.3 >= 0.31 true)
 * @return Boolean.
 */
bool assert_greater_than_equal_float(float greater, float lesser, float leeway)
{
    if (greater - fabsf(leeway) > lesser || greater + fabsf(leeway) > lesser || assert_equal_float_exact_no_print(greater, lesser))
    {
        return true;
    }

    current_test_failed = 1;
    printMsg("\tExpected %f to be greater than or equal to %f  with leeway %f\r\n", greater, lesser, leeway);
    return false;
}

/**
 * @brief Checks integer less than
 * 
 * @param lesser The expected smaller variable
 * @param greater The expected larger variable
 * @return Boolean.
 */
bool assert_less_than_int(int lesser, int greater)
{
    if (lesser < greater)
    {
        return true;
    }

    current_test_failed = 1;
    printMsg("\tExpect %d to be less than %d\r\n", lesser, greater);
    return false;
}

/**
 * @brief Checks integer less than equal to
 * 
 * @param lesser The expected smaller variable
 * @param greater The expected larger variable
 * @return Boolean.
 */
bool assert_less_than_equal_int(int lesser, int greater)
{
    if (lesser <= greater)
    {
        return true;
    }

    current_test_failed = 1;
    printMsg("\tExpect %d to be less than or equal to %d\r\n", lesser, greater);
    return false;
}

/**
 * @brief Checks float less than
 * 
 * @param lesser The expected smaller variable
 * @param greater The expected larger variable
 * @param leeway The allowed margin of error in comparison (i.e. leeway of 0.1 makes 0.3 < 0.3 true)
 * @return Boolean.
 */
bool assert_less_than_float(float lesser, float greater, float leeway)
{

    if (lesser < greater - fabsf(leeway) || lesser < greater + fabsf(leeway) )
    {
        return true;
    }

    current_test_failed = 1;
    printMsg("\tExpected %f to be less than %f with error %f\r\n", lesser, greater, leeway);
    return false;
}

/**
 * @brief Checks float less than or equal to
 * 
 * @param lesser The expected smaller variable
 * @param greater The expected larger variable
 * @param leeway The allowed margin of error in comparison (i.e. leeway of 0.1 makes 0.3 <= 0.2 true)
 * @return Boolean.
 */
bool assert_less_than_equal_float(float lesser, float greater, float leeway)
{
    float test_var = greater - fabsf(leeway);
    if (lesser < greater - fabsf(leeway) || lesser < greater + fabsf(leeway) || assert_equal_float_exact_no_print(lesser, greater)) {
        return true;
    }

    current_test_failed = 1;
    printMsg("\tExpected %f to be less than or equal to %f with error %f\r\n", lesser, greater, leeway);
    return false;
}


