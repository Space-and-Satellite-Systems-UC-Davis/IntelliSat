#ifndef ADC_H
#define ADC_H

#include <stm32l476xx.h>
#include <globals.h>

#define RCC_CCIPR_ADCSEL_SYSCLK (3 << RCC_CCIPR_ADCSEL_Pos)
#define RCC_CCIPR_ADCSEL_PLLSAI1 (2 << RCC_CCIPR_ADCSEL_Pos)
#define RCC_CCIPR_ADCSEL_PLLSAI2 (1 << RCC_CCIPR_ADCSEL_Pos)

uint16_t adc_readVoltage(ADC_TypeDef* adc, int channel);
void adc_init();

#endif
