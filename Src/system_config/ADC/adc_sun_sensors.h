#pragma once

#include <stm32l476xx.h>
#include <globals.h>

#define RCC_CCIPR_ADCSEL_SYSCLK (3 << RCC_CCIPR_ADCSEL_Pos)
#define RCC_CCIPR_ADCSEL_PLLSAI1 (2 << RCC_CCIPR_ADCSEL_Pos)
#define RCC_CCIPR_ADCSEL_PLLSAI2 (2 << RCC_CCIPR_ADCSEL_Pos)


typedef struct PANEL_STRUCT {
    ADC_TypeDef* adcDiode0;
    ADC_TypeDef* adcDiode1;
    int diode0Channel;
    int diode1Channel;
} PANEL;

enum DIODE {DIODE0, DIODE1};


/** Public Functions */

uint16_t adc_readVoltageFromDiode(PANEL* panel, int diodeNumber);
void adc_init();

/** Public Functions */


/** Private functions */

void adc_initializePanelConstants();

bool adc1_isCalibrationDone();
bool adc2_isCalibrationDone(); 
bool adc3_isCalibrationDone();

bool adc_isEOCDownADC1();
bool adc_isEOSDownADC1();

bool adc_isEOCDownADC2();
bool adc_isEOSDownADC2();

bool adc_isEOCDownADC3();
bool adc_isEOSDownADC3();

void ADC1_gpio_init();
void ADC2_gpio_init();
void ADC3_gpio_init();

uint8_t adc_calibrateADC(ADC_TypeDef* adc);

void adcx_initCommon(ADC_TypeDef* adc);

void ADC1_init();
void ADC2_init();
void ADC3_init();

/** Private functions */
