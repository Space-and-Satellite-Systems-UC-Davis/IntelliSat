#ifndef REGISTRY_H
#define REGISTRY_H

#include "../Suites/Suite.h"

typedef struct UT_Registry {
    UT_Suite** suites;
    unsigned int num_suites;
} UT_Registry;

void init_registry(UT_Registry* registry);
void add_suite(UT_Registry* registry, UT_Suite* suite);
void run_registry(UT_Registry* registry);

#endif