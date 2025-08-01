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

extern const float MAX_START_DUTY;
extern const float MAX_DUTY;  // targeted current max duty (should be no higher than 10 for 2ms pulses)
extern const float MID_DUTY;
extern const float MIN_DUTY;  // targeted current min duty (should be no lower than 5 for 1ms pulses)
extern const int PERIOD_uS;  // period is microseconds (5% duty -> min (1ms pulse), 10% duty -> max (2ms pulse))

// init HDD before use
void hdd_init(const PWM_Channels channel);

// if max should be calibrated, the min and max duty, and the max duty previous calibrated
void hdd_calibrate(const PWM_Channels channel, const bool CAL_MAX);

// it seems like to arm, the esc just expects an absolute min duty for 0 after getting some signal
void hdd_arm(PWM_Channels channel);

