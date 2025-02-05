#include "adc_sun_sensors.h"

void ADC1_gpio_init() {
    
}

void ADC2_gpio_init() {
    /**
     * Pan1-pd1 PA0  ADC1(2)_IN5
     * Pan2-pd0 PA1 ADC1(2)_IN6
     * Pan2-pd1 PA2 ADC1(2)_IN7
     * Pan4-pd0 PC0 ADC1(2)3_IN1
     * Pan4-pd1 PC1 ADC1(2)3_IN2
     * Pan5-pd0 PC2 ADC1(2)3_IN3
     * Pan5-pc1 PC3 ADC1(2)3_IN4
     */
    
    RCC->APB2ENR |= RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOCEN;

    GPIOA->MODER |= GPIO_MODER_MODE0 | GPIO_MODER_MODE1 | GPIO_MODER_MODE2;
    GPIOC->MODER |= GPIO_MODER_MODE0 | GPIO_MODER_MODE1 | GPIO_MODER_MODE2 | GPIO_MODER_MODE3;

    GPIOA->PUPDR &= GPIO_PUPDR_PUPD0_Msk | GPIO_PUPDR_PUPD1_Msk | GPIO_PUPDR_PUPD2_Msk;
    GPIOC->PUPDR &= GPIO_PUPDR_PUPD0_Msk | GPIO_PUPDR_PUPD1_Msk | GPIO_PUPDR_PUPD2_Msk | GPIO_PUPDR_PUPD3_Msk;


}

void ADC3_gpio_init() {
    /**
     * Pan0-pd0 PF3 ADC3_IN6
     * Pan0-pd1 PF4 ADC3_IN7
     * Pan1-pd0 PF9 ADC3_IN12
     * Pan3-pd0 PF7 ADC3_IN10
     * Pan3-pd1 PF6 ADC3_IN9
     */
    RCC->APB2ENR |= RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOCEN;
    GPIOF->MODER |= GPIO_MODER_MODE3 | GPIO_MODER_MODE4 | GPIO_MODER_MODE9 | GPIO_MODER_MODE7 | GPIO_MODER_MODE6;
    GPIOF->PUPDR &= GPIO_PUPDR_PUPD3_Msk | GPIO_PUPDR_PUPD4_Msk | GPIO_PUPDR_PUPD9_Msk | GPIO_PUPDR_PUPD7_Msk | GPIO_PUPDR_PUPD6_Msk;
}

void ADC1_init() {
    RCC->APB2ENR |= RCC_AHB2ENR_ADCEN;
}

void ADC2_init() {
    RCC->APB2ENR |= RCC_AHB2ENR_ADCEN;
    // Ensure ADC is disabled before configuring
    if (ADC2->CR & ADC_CR_ADEN) {
        ADC2->CR |= ADC_CR_ADDIS;
        while (ADC2->CR & ADC_CR_ADEN);
    }

    ADC1->CFGR &= ~ADC_CFGR_RES; // Set resolution to 12-bit (00)
    ADC1->CFGR &= ~ADC_CFGR_SCANDIR; // Disable scan mode (single-channel)

    // Configure sampling time
    ADC1->SMPR |= ADC_SMPR_SMP_1; // Set sampling time (12.5 ADC clock cycles)

    // Select Channel 5 (PA0)
    ADC1->CHSELR = ADC_CHSELR_CHSEL5; // Only enable Channel 5

    // Enable ADC
    ADC1->CR |= ADC_CR_ADEN;
    while (!(ADC1->ISR & ADC_ISR_ADRDY)); // Wait for ADC to be ready
}

void ADC3_init() {
    RCC->APB2ENR |= RCC_AHB2ENR_ADCEN;
}