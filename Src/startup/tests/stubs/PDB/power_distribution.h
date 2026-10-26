#pragma once
#include <stdbool.h>
/* Electrical eligibility is a modeled boundary, not a voltage qualification. */
#ifndef STARTUP_TEST_DEFAULT_BATTERY
bool startup_test_battery_eligible(void);
#define STARTUP_BATTERY_ELIGIBLE() startup_test_battery_eligible()
#endif
/* Hardware boundary: Src/peripherals/PDB/power_distribution.h. */
void pdb_init(void);
void pdb_pyro_on(void);
void pdb_pyro_off(void);
