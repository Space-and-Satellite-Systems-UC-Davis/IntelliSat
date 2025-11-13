#pragma once
#include "IMU/ASM330LHH.h"

void test_initial_state();

bool test_stable_while_flat(float max_gyro_fluctuation, int rate, int seconds);