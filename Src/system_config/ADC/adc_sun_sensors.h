#ifndef ADC_H
#define ADC_H

#include <stm32l476xx.h>
#include <globals.h>

#define RCC_CCIPR_ADCSEL_SYSCLK (3 << RCC_CCIPR_ADCSEL_Pos)
#define RCC_CCIPR_ADCSEL_PLLSAI1 (2 << RCC_CCIPR_ADCSEL_Pos)
#define RCC_CCIPR_ADCSEL_PLLSAI2 (1 << RCC_CCIPR_ADCSEL_Pos)

typedef struct PANEL_STRUCT {
    ADC_TypeDef* adcDiode0;
    ADC_TypeDef* adcDiode1;
    int diode0Channel;
    int diode1Channel;
} PANEL_INFO;

typedef enum DIODE_ENUM {DIODE0, DIODE1} DIODES;


typedef enum PANELS_ENUM {
    PANEL0,
    PANEL1,
    PANEL2,
    PANEL3,
    PANEL4,
    PANEL5
} PANELS;


uint16_t adc_readVoltageFromDiode(PANELS panel, DIODES diodeNumber);
void adc_init();

#endif
