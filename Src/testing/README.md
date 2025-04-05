# Unit Testing

This is the directory that contains the Unity files and the code that runs the unit tests, including `run_unit_tests`.

[Unity Docs](https://www.throwtheswitch.org/unity)

### Adding a unit test

#### Step 1. Add the test code

Add the test code to `Src/testing/test.c`

```c
void test_basic_example_unity() {
    TEST_ASSERT(1);

    // Try to use type specific functions as much as possible, as they produce
    // better debugging output!
    TEST_ASSERT_EQUAL_INT(1, 1);
}
```

Then add the function header to the `Src/testing/test.h` file.

```c
void test_basic_example_unity();
```

#### Step 2. Get Unity to run the test code

Get the unit testing to run the code with the `RUN_TEST` function.

```c
// NOTE: These are UNIT tests not related to ADCS
int run_unit_tests() {
    UNITY_BEGIN();

	// NOTE: Add all tests with the RUN_TEST function
    RUN_TEST(test_basic_example_unity);

    return UNITY_END();
}
```

#### Step 3. Run the unit testing

If you want Unity to run the tests, change the macro definition of `RUN_UNIT_TESTS` to `1`.

```c
// ...

#define RUN_UNIT_TESTS	1

int main() {
    init_init();
    init_platform(!RUN_TEST);
    // ^ don't want to run the Scheduler in case we are running other tests

#if (RUN_UNIT_TESTS==1)
    run_unit_tests();
#endif

// ...
```
