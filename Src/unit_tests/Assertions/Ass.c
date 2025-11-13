#include "Ass.h"

#include <string.h>
#include <math.h>
#include <stdio.h>
#include <LED/led.h>
#include <inttypes.h>

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

bool assert_equal(int actual, int expected)
{
    return actual == expected;
}

// equal
bool assert_equal_int(int actual, int expected)
{
    if (actual != expected)
    {
        printMsg("Fail. Expected integer %d to equal %d\r\n", actual, expected);
        return false;
    }

    printMsg("Pass.\r\n");
    return true;
}

bool assert_equal_float_exact(float actual, float expected)
{
    uint32_t actual_bits;
    uint32_t expected_bits;

    memcpy(&actual_bits, &actual, sizeof(float));
    memcpy(&expected_bits, &expected, sizeof(float));

    if (actual_bits != expected_bits)
    {
        printMsg("Fail. Expected float %f (bits %" PRIu32 ") to equal float %f (bits %" PRIu32 ") exactly.\r\n", actual, actual_bits, expected, expected_bits);
        return false;
    }

    printMsg("Pass.\r\n");
    return true;
}

bool assert_equal_float_within(float actual, float expected, float leeway)
{
    // allow leeway to prevent floating point precision errors

    if (!(fabsf(actual - expected) <= leeway))
    {
        printMsg("Fail. Expected float %f to equal %f with error %f\r\n", actual, expected, leeway);
        return false;
    }

    printMsg("Pass.\r\n");
    return true;
}

// char array
bool assert_equal_char_array(char *actual, char *expected, int length)
{
    for (int i = 0; i < length; i++)
    {
        if (actual[i] != expected[i])
        {
            printMsg("Fail. \'");
            for (int j = 0; j < length; j++)
            {
                printMsg("%c", actual[j]);
            }
            printMsg("\' not equivalent to \'");
            for (int j = 0; j < length; j++)
            {
                printMsg("%c", expected[j]);
            }
            printMsg("\'.\r\n");
            return false;
        }
    }

    printMsg("Pass.\r\n");
    return true;
}

// greater than int
bool assert_greater_than_int(int greater, int lesser)
{
    if (greater > lesser)
    {
        printMsg("Pass.\r\n");
        return true;
    }

    printMsg("Fail. Expected int %d to be greater than %d\r\n", greater, lesser);
    return false;
}

bool assert_greater_than_equal_int(int greater, int lesser)
{
    if (greater >= lesser)
    {
        printMsg("Pass.\r\n");
        return true;
    }

    printMsg("Fail. Expected int %d to be greater than or equal to %d\r\n", greater, lesser);
    return false;
}

// greater than float
bool assert_greater_than_float(float greater, float lesser, float leeway)
{
    if (greater - fabsf(leeway) > lesser)
    {
        printMsg("Pass.\r\n");
        return true;
    }

    printMsg("Fail. Expected float %f to be greater than %f with leeway %f\r\n", greater, lesser, leeway);
    return false;
}

bool assert_greater_than_equal_float(float greater, float lesser, float leeway)
{
    if (greater - fabsf(leeway) > lesser || assert_equal_float_exact_no_print(greater, lesser))
    {
        printMsg("Pass.\r\n");
        return true;
    }

    printMsg("Fail. Expected float %f to be greater than or equal to %f  with leeway %f\r\n", greater, lesser, leeway);
    return false;
}

// less than int
bool assert_less_than_int(int lesser, int greater)
{
    if (lesser < greater)
    {
        printMsg("Pass.\r\n");
        return true;
    }

    printMsg("Fail. Expect int %d to be less than %d\r\n", lesser, greater);
    return false;
}

bool assert_less_than_equal_int(int lesser, int greater)
{
    if (lesser <= greater)
    {
        printMsg("Pass.\r\n");
        return true;
    }

    printMsg("Fail. Expect int %d to be less than or equal to %d\r\n", lesser, greater);
    return false;
}

// less than float
bool assert_less_than_float(float lesser, float greater, float leeway)
{
    if (lesser < greater - fabsf(leeway))
    {
        printMsg("Pass.\r\n");
        return true;
    }

    printMsg("Fail. Expect float %f to be less than %f with error %f\r\n", lesser, greater, leeway);
    return false;
}

bool assert_less_than_equal_float(float lesser, float greater, float leeway)
{
    if (lesser < greater - fabsf(leeway) || assert_equal_float_exact_no_print(lesser, greater)) {
        printMsg("Pass.\r\n");
        return true;
    }

    printMsg("Fail. Expect float %f to be less than or equal to %f with error %f\r\n", lesser, greater, leeway);
    return false;
}


