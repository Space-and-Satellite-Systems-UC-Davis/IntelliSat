/*
 * For functions necessary to initialize the esc or
 * hdd prior to activation
 */

#pragma once

#include <Timers/timers.h>
#include <ADC/adc.h>
#include <LED/led.h>
#include <inttypes.h>
#include <stdbool.h>

// if max should be calibrated, the min and max duty, and the max duty previous calibrated
void calibrate(const bool CAL_MAX, const float MIN_DUTY, const float MAX_DUTY, const float MAX_START_DUTY);

// it seems like to arm, the esc just expects an absolute min duty for 0 after getting some signal
void arm(const float MIN_DUTY, const float MAX_DUTY);

