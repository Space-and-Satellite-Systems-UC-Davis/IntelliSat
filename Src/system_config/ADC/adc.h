#ifndef ADC_H
#define ADC_H

#include <stm32l476xx.h>
#include <globals.h>

#define INTERNAL_VOLTAGE_REFERENCE 2.048
#define MAX_12_BIT_READING 4095.0

#define RCC_CCIPR_ADCSEL_SYSCLK (3 << RCC_CCIPR_ADCSEL_Pos)
#define RCC_CCIPR_ADCSEL_PLLSAI1 (2 << RCC_CCIPR_ADCSEL_Pos)
#define RCC_CCIPR_ADCSEL_PLLSAI2 (1 << RCC_CCIPR_ADCSEL_Pos)

uint16_t adc_readChannel(ADC_TypeDef* adc, int channel);
float adc_readVoltage(uint16_t channelReading);
void set_continuous_dma(ADC_TypeDef* adc, int channel);
void adc_init();


#endif
