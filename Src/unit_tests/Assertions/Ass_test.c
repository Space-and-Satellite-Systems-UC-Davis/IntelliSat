#include "Ass.h"


void testFunction_AssTest() {
	printMsg("\r\n\r\n---- Starting Unit Tests ----\r\n");
    assert_equal_int(2, 2);
    assert_equal_int(2, 3);

    assert_equal_float_exact(0.1f, 0.1f);
    assert_equal_float_exact(0.1f, 0.3f - 0.2f);

    assert_equal_float_within(0.001f, 0.002f, 0.001f);
    assert_equal_float_within(0.001f, 0.002f - 0.001f, 0.0001f);
    assert_equal_float_within(0.001f, 0.002f, 0.0005f);

    assert_equal_char_array("hello", "hello", 5);
    assert_equal_char_array("hello", "hellp", 5);
    assert_equal_char_array("helo", "helop", 4);

    assert_greater_than_int(5, 4);
    assert_greater_than_int(5, 5);
    assert_greater_than_int(5, 6);

    
    assert_greater_than_equal_int(5, 4);
    assert_greater_than_equal_int(5, 5);
    assert_greater_than_equal_int(5, 6);

    assert_greater_than_float(5.0f, 4.0f, 0.01f);
    assert_greater_than_float(5.0f, 5.0f, 0.01f);
    assert_greater_than_float(5.0f, 6.0f, 0.01f);

    assert_greater_than_equal_float(5.0f, 4.0f, 0.01f);
    assert_greater_than_equal_float(5.0f, 5.0f, 0.01f);
    assert_greater_than_equal_float(5.0f, 6.0f, 0.01f);

    assert_less_than_int(5, 4);
    assert_less_than_int(5, 5);
    assert_less_than_int(5, 6);

    assert_less_than_equal_int(5, 4);
    assert_less_than_equal_int(5, 5);
    assert_less_than_equal_int(5, 6);
    

    assert_less_than_float(5.0f, 4.0f, 0.01f);
    assert_less_than_float(5.0f, 5.0f, 0.01f);
    assert_less_than_float(5.0f, 6.0f, 0.01f);
    
    assert_less_than_equal_float(5.0f, 4.0f, 0.01f);
    assert_less_than_equal_float(5.0f, 5.0f, 0.01f);
    assert_less_than_equal_float(5.0f, 6.0f, 0.01f);

}
