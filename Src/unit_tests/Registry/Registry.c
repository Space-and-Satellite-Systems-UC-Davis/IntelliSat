#include "Registry.h"
#include "../Suites/suite.h"
#include "print_scan.h"
#include <stdlib.h>

void add_suite(UT_Registry *registry, UT_Suite *suite)
{
    UT_Suite* new_suites = realloc(registry->suites, (registry->num_suites + 1) * sizeof(UT_Suite));
    registry->suites = new_suites;
    registry->suites[registry->num_suites] = suite;

    registry->num_suites++;
    printMsg("num suites %d", registry->num_suites);
}

void run_registry(UT_Registry *registry)
{
    for (unsigned int i = 0; i < registry->num_suites; i++) {
        run_suite(registry->suites[i]);
    }
}

void init_registry(UT_Registry *registry)
{
    registry->suites = NULL;
    registry->num_suites = 0;
    printMsg("Registry initialized");
}
