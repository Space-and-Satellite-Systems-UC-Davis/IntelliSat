#include "globals.h"
#include "IMU_Tests/IMU_Tests.h"
#include "Suites/suite.h"
#include "print_scan.h"
#include "Assertions/Ass.h"
#include "Registry/Registry.h"



void run_tests()
{

    // IMU Tests
    //  test_initial_state();
    UT_Suite suite;
    init_suite(&suite, 1, "Initial state");
    add_test(&suite, test_stable_while_flat, "Stable while flat");


    // set up registry
    UT_Registry registry;
    init_registry(&registry);

    add_suite(&registry, &suite);
    
    run_registry(&registry);

}
