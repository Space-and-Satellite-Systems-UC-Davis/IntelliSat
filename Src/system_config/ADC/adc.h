// /*
//  * adc.h
//  *
//  *  Created on: Feb 10, 2024
//  *      Author: Chandrark Muddana
//  */

// #ifndef SYSTEM_CONFIG_ADC_ADC_H_
// #define SYSTEM_CONFIG_ADC_ADC_H_
// #include "stm32l476xx.h"
// #include <globals.h>
// #include <GPIO/gpio.h>
// #include "../../tools/print_scan.h"

// #define RCC_CCIPR_ADCSEL_SYSCLK (3 << RCC_CCIPR_ADCSEL_Pos)
// #define GPIO_MODER_MODE0_ANALOG (3 << GPIO_MODER_MODE0_Pos)
// #define ADC_SQR1_SQ1_CHAN18_AS_1st_CONV (18 << ADC_SQR1_SQ1_Pos)
// #define ADC_CHAN18_640_5_CLK_CYC (7 << ADC_SMPR2_SMP18_Pos)

// //CALL THESE IN ORDER

// //Initializes ADC clocks
// void adc_init();

// //Enables the ADC
// void adc_enable();

// //Configures the GPIO pin C0 for the ADC
// void adc_configGPIO();

// //Configures the channel to set it to pin C0, and sets sampling time to 640.5 cycles per sample
// void adc_setChannel();

// //Test function
// void adc_setConstantGPIOValue();

// /* Gets a value from the ADC
//  * @returns: The value from the ADC
//  * 		it is from 0 to 4095
//  * 		symbolizes 0.0V to 3.3V
//  */
// uint16_t adc_singleConversion();

// /* Converts the adcVal to a millivolt value using 2.048 VREFINT (VREFBUF->CSR VRS bit is 0)
//  * @returns: the millivolt value of an int
//  */
// uint16_t adc_adcToVolt1(uint16_t adcVal);

// /* Converts the adcVal to a millivolt value using 2.5 VREFINT (VREFBUF->CSR VRS bit is 1)
//  * @returns: the millivolt value of an int
//  */
// uint16_t adc_adcToVolt2(uint16_t adcVal);

// /* Converts the adcVal to a millivolt value using the battery internal reference (idk why its different)
//  * @returns: the millivolt value of an int
//  */
// uint16_t adc_adcToBatVolt(uint16_t adcVal);

// /* Prints the millivolt value in a volt fashion
//  *  @returns: none
//  */
// void adc_printVolt(uint16_t volt);

// void adc_printMilliVolt(uint16_t volt);

// #endif /* SYSTEM_CONFIG_ADC_ADC_H_ */
