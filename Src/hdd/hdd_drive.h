/*
 * Any functions or code relevant to directly driving the
 * rotation of the hdd
 */

#pragma once

#include "hdd_init.h"

// default ramp
void hddRamp(const PWM_Channels pwm, const dutyType TARGET_DUTY, int8_t doPrint);

// will drive to the targeted duty without slipping; will ramp as needed
void hddDrive(const PWM_Channels pwm, const dutyType TARGET_DUTY, int8_t doPrint);

// tries to avoid slipping while not blocking program execution
void hddDriveNB(const PWM_Channels pwm, dutyType TARGET_DUTY);

// reverse ramp
void hddRampRev(const float TARGET_DUTY, const float MIN_DUTY, const float MAX_DUTY);

// sets the duty below the minimum the esc will accept through toggling of power
void hddSetSubMinDuty(const float PERCENT, const float MIN_DUTY, const float ZERO_DUTY, const uint64_t TIME_MS);
