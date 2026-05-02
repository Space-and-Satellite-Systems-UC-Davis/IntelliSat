#include "Registry.h"
#include "../Suites/suite.h"
#include "print_scan.h"
#include <stdlib.h>

/**
 * @brief Add a suite to the registry
 * 
 * @param registry The registry to add the suite to
 * @param suite The suite to be added
 * @return None
 */
void add_suite(UT_Registry *registry, UT_Suite *suite)
{
    UT_Suite* new_suites = realloc(registry->suites, (registry->num_suites + 1) * sizeof(UT_Suite));
    registry->suites = new_suites;
    registry->suites[registry->num_suites] = suite;

    registry->num_suites++;
    printMsg("num suites %d", registry->num_suites);
}

/**
 * @brief Run a registry. Can be called by itself
 * 
 * @param registry The registry to run
 * @return None
 */
void run_registry(UT_Registry *registry)
{
    for (unsigned int i = 0; i < registry->num_suites; i++) {
        run_suite(registry->suites[i]);
    }
}


/**
 * @brief Initializes default values for a registry
 * 
 * Realistically, only one registry needs to be registered ever because you can 
 * just add a bunch of suites to it.
 * 
 * @param registry The UT_Registry variable
 * @return None
 */
void init_registry(UT_Registry *registry)
{
    registry->suites = NULL;
    registry->num_suites = 0;
    printMsg("Registry initialized");
}
