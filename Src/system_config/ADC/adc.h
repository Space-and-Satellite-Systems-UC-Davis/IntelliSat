/*
 * adc.h
 *
 *  Created on: Feb 10, 2024
 *      Author: Chandrark Muddana
 */

#ifndef SYSTEM_CONFIG_ADC_ADC_H_
#define SYSTEM_CONFIG_ADC_ADC_H_
#include "stm32l476xx.h"

//CALL THESE IN ORDER

//Initializes ADC clocks
void adc_init();

//Enables the ADC
void adc_enable();

//Configures the GPIO pin A0 for the ADC
void adc_configGPIO();

//Configures the channel to set it to pin A0, and sets sampling time to 640.5 cycles per sample
void adc_setChannel();

/* Gets a value from the ADC
 * @returns: The value from the ADC
 * 		it is from 0 to 4095
 * 		symbolizes 0.0V to 3.3V
 */
uint16_t adc_singleConversion();

#endif /* SYSTEM_CONFIG_ADC_ADC_H_ */
