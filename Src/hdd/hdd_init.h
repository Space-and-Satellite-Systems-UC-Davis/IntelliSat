/*
 * For functions necessary to initialize the esc or
 * hdd prior to activation
 */

#pragma once

#include <Timers/timers.h>
#include <ADC/adc.h>
#include <LED/led.h>
#include <inttypes.h>
#include <print_scan.h>

typedef uint8_t dutyType;
extern const dutyType MAX_START_DUTY;
extern const dutyType SLIP_DUTY;
extern const dutyType MAX_DUTY;  // targeted current max duty (should be no higher than 10 for 2ms pulses)
extern const dutyType MID_DUTY;
extern const dutyType MIN_DUTY;  // targeted current min duty (should be no lower than 5 for 1ms pulses)
extern const uint32_t SLIP_TIME_MS;
extern const uint32_t JUMP_TIME_MS;
extern const int PERIOD_uS;  // period is microseconds (5% duty -> min (1ms pulse), 10% duty -> max (2ms pulse))

// init HDD before use
void hdd_init(const PWM_Channels channel);

// if max should be calibrated, the min and max duty, and the max duty previous calibrated
void hdd_calibrate(const PWM_Channels channel, const int CAL_MAX);

// it seems like to arm, the esc just expects an absolute min duty for 0 after getting some signal
void hdd_arm(PWM_Channels channel);

