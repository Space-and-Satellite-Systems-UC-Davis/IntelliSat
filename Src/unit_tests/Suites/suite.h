typedef void (*Test_Fun)();

typedef struct UT_Suite {
    unsigned int id;
    char* name;
    Test_Fun* tests;
    unsigned int num_tests;
} UT_Suite;

void init_suite(UT_Suite* suite, unsigned int id, char* name);
void run_suite(UT_Suite* suite);
void add_test(UT_Suite* suite, Test_Fun test_function);