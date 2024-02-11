/*
 * adc.c
 *
 *  Created on: Feb 10, 2024
 *      Author: Chandrark Muddana
 */

#include "adc.h"

//USES ADC1

void adc_init(){
	RCC->APB2ENR |= RCC_AHB2ENR_ADCEN; //enables ADC clock
	RCC->CCIPR   &= ~( RCC_CCIPR_ADCSEL ); //enables peripheral clock
	RCC->CCIPR   |=  ( 3 << RCC_CCIPR_ADCSEL_Pos ); //Sets ADC clock to system clock
	ADC1->CR    &= ~( ADC_CR_DEEPPWD );//makes sure ADC isn't in deep power down mode
	ADC1->CR |= ADC_CR_ADVREGEN; //enables ADC voltage regulator
	nop(10000); //waits a bit
	ADC1->CR |= ADC_CR_ADCAL; //Calibrates ADC
	while ((ADC1->CR & ADC_CR_ADCAL) != 0) { } //Waits until ADC is calibrated

}

void adc_enable(){
	ADC1->ISR |= ADC_ISR_ADRDY; // Checks if it is ready
	ADC1->CR |= ADC_CR_ADEN; //Enables ADC
	while ((ADC1->ISR & ADC_ISR_ADRDY) == 0) { } //Waits until ADRDY is reset
}
void adc_configGPIO(){
	GPIOA->OTYPER       &= ~( GPIO_OTYPER_OT0 ); //Reset A0 pin
	GPIOA->PUPDR        &= ~( GPIO_OSPEEDR_OSPEED0 ); //Reset A0 pin
	GPIOA->OSPEEDR      &= ~( GPIO_PUPDR_PUPD0 ); //Reset A0 pin
	GPIOA->MODER        &= ~( GPIO_MODER_MODE0 ); //Reset A9 pin
	GPIOA->MODER        |=  ( 0x3 << GPIO_MODER_MODE0_Pos ); //Sets mode to analog
}

void adc_setChannel(){
	//leave sampling time at default
	ADC1->SQR1 &= ~( ADC_SQR1_L ); //Sets number of channels in the sequence of 1
	ADC1->SQR1 &= ~(ADC_SQR1_SQ1); //Resets the sequence
	ADC1->SQR1 |= (5 << ADC_SQR1_SQ1_Pos); //Sets the sequence to just channel 5 (PIN A0)
	ADC1->SMPR1 &= ~( ADC_SMPR1_SMP5 ); //Resets the sampling time of channel 5
	ADC1->SMPR1 |=  ( 0x7 << ADC_SMPR1_SMP5_Pos ); //Sets the sampling time to max: 640.5 cycles per sample
}

// Perform a single ADC conversion.
// (Assumes that there is only one channel per sequence)
uint16_t adc_singleConversion() {
  // Start the ADC conversion.
  ADC1->CR  |=  ( ADC_CR_ADSTART );
  // Wait for the 'End Of Conversion' flag.
  while ( !( ADC1->ISR & ADC_ISR_EOC ) ) {};
  // Read the converted value (this also clears the EOC flag).
  uint16_t adc_val = ADC1->DR;
  // Wait for the 'End Of Sequence' flag and clear it.
  while ( !( ADC1->ISR & ADC_ISR_EOS ) ) {};
  ADC1->ISR |=  ( ADC_ISR_EOS );
  // Return the ADC value.
  return adc_val;
}
