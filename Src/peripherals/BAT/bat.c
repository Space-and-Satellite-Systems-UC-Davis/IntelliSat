/*
 * bat.c
 *
 * Created on: May 21, 2024
 *     Author: Winson Yu
 */

#include "bat.h"

// Macros
#define THRESHOLD_CRIT 3
#define THRESHOLD_SAFE 3.3

// initialize the ADC
void bat_initMonitoring(){
	adc_init();
	adc_enable();
	adc_setChannel();
}

uint16_t bat_getVoltageValue(){
	// get the value from the ADC
	uint16_t adcVal = adc_singleConversion();

	// normalize the ADC value into volts and return
	return adc_adcToBatVolt(adcVal);
}

bool bat_isCrit(){
	return bat_getVoltageValue() < THRESHOLD_CRIT;
}

bool bat_isSafe(){
	return bat_getVoltageValue() > THRESHOLD_SAFE;
}

