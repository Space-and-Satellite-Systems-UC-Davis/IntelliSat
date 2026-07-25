#ifndef REGISTRY_H
#define REGISTRY_H

#include "../Suites/Suite.h"

typedef struct UT_Registry {
    UT_Suite** suites;
    unsigned int num_suites;
} UT_Registry;

/**
 * @brief Initializes default values for a registry
 * 
 * Realistically, only one registry needs to be registered ever because you can 
 * just add a bunch of suites to it.
 * 
 * @param registry The UT_Registry variable
 * @return None
 */
void init_registry(UT_Registry* registry);
/**
 * @brief Add a suite to the registry
 * 
 * @param registry The registry to add the suite to
 * @param suite The suite to be added
 * @return None
 */
void add_suite(UT_Registry* registry, UT_Suite* suite);
/**
 * @brief Run a registry. Can be called by itself
 * 
 * @param registry The registry to run
 * @return None
 */
void run_registry(UT_Registry* registry);

#endif