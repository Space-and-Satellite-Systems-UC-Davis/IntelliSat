/*
 * bat.h
 *
 * Created on: May 21, 2024
 *     Author: Winson Yu
 */

#ifndef REALOP1_PERIPHERALS_BAT_H_
#define REALOP1_PERIPHERALS_BAT_H_
#include <globals.h>
#include <ADC/adc.h>

// initialize the ADC
void bat_initMonitoring();

// returns battery voltage returned by ADC functions
uint16_t bat_getVoltageValue();

// returns if the battery state is under the critical threshold
bool bat_isCrit();

// returns if the battery state is above the safe threshold
bool bat_isSafe();

#endif
