/*
 * Any functions or code relevant to directly driving the
 * rotation of the hdd
 */

#pragma once

#include <Timers/timers.h>
#include <ADC/adc.h>
#include <LED/led.h>
#include <inttypes.h>
#include <stdbool.h>

// default ramp
void ramp(const float TARGET_DUTY, const float MIN_DUTY, const float MAX_DUTY);

// reverse ramp
void rampRev(const float TARGET_DUTY, const float MIN_DUTY, const float MAX_DUTY);

// sets the duty below the minimum the esc will accept through toggling of power
void setSubMinDuty(const float PERCENT, const float MIN_DUTY, const float ZERO_DUTY, const uint64_t TIME_MS);
