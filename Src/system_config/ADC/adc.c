#include "adc.h"


/** Private helper functions */

static bool adc1_isCalibrationDone() { return (ADC1->CR & ADC_CR_ADCAL) != 0; }
static bool adc2_isCalibrationDone() { return (ADC2->CR & ADC_CR_ADCAL) != 0; }
static bool adc3_isCalibrationDone() { return (ADC3->CR & ADC_CR_ADCAL) != 0; }

static bool adc_isEOCDownADC1() { return !(ADC1->ISR & ADC_ISR_EOC); }
static bool adc_isEOSDownADC1() { return !(ADC1->ISR & ADC_ISR_EOS); }

static bool adc_isEOCDownADC2() { return !(ADC2->ISR & ADC_ISR_EOC); }
static bool adc_isEOSDownADC2() { return !(ADC2->ISR & ADC_ISR_EOS); }

static bool adc_isEOCDownADC3() { return !(ADC3->ISR & ADC_ISR_EOC); }
static bool adc_isEOSDownADC3() { return !(ADC3->ISR & ADC_ISR_EOS); }

static bool adc_isADRDYNotResetADC1() { return (ADC1->ISR & ADC_ISR_ADRDY) == 0; }
static bool adc_isADRDYNotResetADC2() { return (ADC2->ISR & ADC_ISR_ADRDY) == 0; }
static bool adc_isADRDYNotResetADC3() { return (ADC3->ISR & ADC_ISR_ADRDY) == 0; }

static void adc_enable(ADC_TypeDef* adc){
	adc->ISR |= ADC_ISR_ADRDY; // Set before enabling ADC
	adc->CR |= ADC_CR_ADEN; //Enables ADC

    switch((int) adc) {
        case (int) ADC1:
            wait_with_timeout(adc_isADRDYNotResetADC1, DEFAULT_TIMEOUT_MS); //Waits until ADRDY is reset
            break;
        case (int) ADC2:
            wait_with_timeout(adc_isADRDYNotResetADC2, DEFAULT_TIMEOUT_MS); //Waits until ADRDY is reset
            break;
        case (int) ADC3:
            wait_with_timeout(adc_isADRDYNotResetADC3, DEFAULT_TIMEOUT_MS); //Waits until ADRDY is reset
            break;
    }
}

static void ADC1_gpio_init() {
    // Currently not being used
}

static void ADC2_gpio_init() {
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

    GPIOA->ASCR |= GPIO_ASCR_ASC0 | GPIO_ASCR_ASC1 | GPIO_ASCR_ASC2;
    GPIOC->ASCR |= GPIO_ASCR_ASC0 | GPIO_ASCR_ASC1 | GPIO_ASCR_ASC2 | GPIO_ASCR_ASC3; 

}

static void ADC3_gpio_init() {
    /**
     * Pan0-pd0 PF3 ADC3_IN6
     * Pan0-pd1 PF4 ADC3_IN7
     * Pan1-pd0 PF9 ADC3_IN12
     * Pan3-pd0 PF7 ADC3_IN10
     * Pan3-pd1 PF6 ADC3_IN9
     */
    RCC->APB2ENR |= RCC_AHB2ENR_GPIOFEN;
    GPIOF->MODER |= GPIO_MODER_MODE3 | GPIO_MODER_MODE4 | GPIO_MODER_MODE9 | GPIO_MODER_MODE7 | GPIO_MODER_MODE6;
    GPIOF->PUPDR &= GPIO_PUPDR_PUPD3_Msk | GPIO_PUPDR_PUPD4_Msk | GPIO_PUPDR_PUPD9_Msk | GPIO_PUPDR_PUPD7_Msk | GPIO_PUPDR_PUPD6_Msk;
    GPIOF->ASCR |= GPIO_ASCR_ASC3 | GPIO_ASCR_ASC4 | GPIO_ASCR_ASC6 | GPIO_ASCR_ASC7 | GPIO_ASCR_ASC9;
}

static uint8_t adc_calibrateADC(ADC_TypeDef* adc) {
    adc->CR &= ~ADC_CR_DEEPPWD;//makes sure ADC isn't in deep power down mode
	adc->CR |= ADC_CR_ADVREGEN; //enables ADC voltage regulator

	nop(10000);
    adc->CR &= ~ADC_CR_ADEN; //Disable ADC
	adc->CR &= ~ADC_CR_ADCALDIF; //Single-ended input
    adc->CR |= ADC_CR_ADCAL; //Enable Calibration



    //Wait for Calibration to Finish
    switch ((int)adc) {
        case (int)ADC1:
            wait_with_timeout(adc1_isCalibrationDone, DEFAULT_TIMEOUT_MS);
            break;
        case (int)ADC2:
            wait_with_timeout(adc2_isCalibrationDone, DEFAULT_TIMEOUT_MS);
            break;
        case (int)ADC3:
            wait_with_timeout(adc3_isCalibrationDone, DEFAULT_TIMEOUT_MS);
            break;
        default:
            break;
    }

    return adc->CALFACT | ADC_CALFACT_CALFACT_S;
}

static void adcx_initCommon(ADC_TypeDef* adc) {
    adc_calibrateADC(adc);
    adc->SQR1 &= ~( ADC_SQR1_L ); //Sets number of channels in the sequence of 1
	adc->SQR1 &= ~(ADC_SQR1_SQ1); //Resets the sequence
}

static void ADC1_init() {
    ADC1_gpio_init();
    adcx_initCommon(ADC1);
    //Not being set because need to know what pins being used and ADC1 is currently not being used

}

static void ADC2_init() {
    ADC2_gpio_init();
    adcx_initCommon(ADC2);
    ADC2->SMPR1 = 0; 

    //Set sample rate to slowest rate (640.5) per conversion 
    ADC2->SMPR1 |=  ADC_SMPR1_SMP1 | ADC_SMPR1_SMP2  | ADC_SMPR1_SMP3 | ADC_SMPR1_SMP4 | ADC_SMPR1_SMP5 | ADC_SMPR1_SMP6 | ADC_SMPR1_SMP7;
    adc_enable(ADC2);
}

static void ADC3_init() {
    ADC3_gpio_init();
    adcx_initCommon(ADC3);

    //Set sample rate to slowest rate (640.5) per conversion 
    ADC3->SMPR1 = 0; 
    ADC3->SMPR1 |=  ADC_SMPR1_SMP6 | ADC_SMPR1_SMP7  | ADC_SMPR1_SMP9;
    ADC3->SMPR2 = 0;
    ADC3->SMPR2 |= ADC_SMPR2_SMP10 | ADC_SMPR2_SMP12; 

    adc_enable(ADC3);
}


/** Private helper functions */


/** Public Functions */

void adc_init() {
    RCC->AHB2ENR |= RCC_AHB2ENR_ADCEN;
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    
    RCC->CCIPR &= ~RCC_CCIPR_ADCSEL; //enables peripheral clock
	RCC->CCIPR |= RCC_CCIPR_ADCSEL_SYSCLK; //Sets ADC clock to system clock

    ADC1_init();
    ADC2_init();
    ADC3_init();
    ADC123_COMMON->CCR |= ADC_CCR_VREFEN;

    VREFBUF->CSR |= VREFBUF_CSR_ENVR; //Enables internal reference buffer
	VREFBUF->CSR &= ~(VREFBUF_CSR_HIZ); //Set to internal voltage reference mode (this is default high)

	while(!(VREFBUF->CSR & VREFBUF_CSR_VRR)); //Waits until voltage reference value reaches expected output
	VREFBUF->CSR |= VREFBUF_CSR_VRS; //Sets internal reference buffer to around 2.5V


}

uint16_t adc_readChannel(ADC_TypeDef* adc, int channel) {
   adc->CR &= ~ADC_CR_ADSTART;
   adc->SQR1 |= channel << ADC_SQR1_SQ1_Pos; //Set the channel in sequence to be converted
    
    adc->CR  |=  ( ADC_CR_ADSTART ); //Start conversion

	// Wait for the 'End Of Conversion' flag.
    switch((int)adc) {
        case (int) ADC1:
            wait_with_timeout(adc_isEOCDownADC1, DEFAULT_TIMEOUT_MS);    
            break;
        case (int) ADC2:
            wait_with_timeout(adc_isEOCDownADC2, DEFAULT_TIMEOUT_MS);    
            break;
        case (int) ADC3:
            wait_with_timeout(adc_isEOCDownADC3, DEFAULT_TIMEOUT_MS);    
            break;
    }
	// Read the converted value (this also clears the EOC flag).
	uint16_t adc_val = adc->DR;

	// Wait for the 'End Of Sequence' flag and clear it.
	switch((int)adc) {
        case (int) ADC1:
            wait_with_timeout(adc_isEOSDownADC1, DEFAULT_TIMEOUT_MS);    
            break;
        case (int) ADC2:
            wait_with_timeout(adc_isEOSDownADC2, DEFAULT_TIMEOUT_MS);    
            break;
        case (int) ADC3:
            wait_with_timeout(adc_isEOSDownADC3, DEFAULT_TIMEOUT_MS);    
            break;
    }
	adc->ISR |=  ( ADC_ISR_EOS );

    adc->SQR1 &= ~(channel << ADC_SQR1_SQ1_Pos); //Remove the channel from the sequence

	// Return the ADC value.
	return adc_val;
}

float adc_readVoltage(uint16_t channelReading) {
    return (channelReading / MAX_12_BIT_READING) * INTERNAL_VOLTAGE_REFERENCE;
}

void adc_continuousDMAStart(ADC_TypeDef* adc, int channel) {
	adc->CR &= ~ADC_CR_ADSTART; //Pause

	adc->SQR1 |= channel << ADC_SQR1_SQ1_Pos; //Set the channel in sequence to be converted

	adc->CFGR  |=  ( ADC_CFGR_CONT ); //Continuous conversion
	adc->CFGR  |=  ( ADC_CFGR_OVRMOD ); //If haven't read DR in time, overwrite
	adc->CFGR  |=  ( ADC_CFGR_DMACFG ); //Enable DMA circular mode
	adc->CFGR  |=  ( ADC_CFGR_DMAEN ); //Enable DMA, ADC-side

	adc->CR  |=  ( ADC_CR_ADSTART ); //Start conversion
}
void adc_continuousDMAStop(ADC_TypeDef* adc, int channel) {
	adc->CR &= ~ADC_CR_ADSTART; //Pause

	//Unset the channel in sequence to be converted
	adc->SQR1 &= ~(channel << ADC_SQR1_SQ1_Pos);

	adc->CFGR  &=  ~( ADC_CFGR_CONT );
	adc->CFGR  &=  ~( ADC_CFGR_OVRMOD );
	adc->CFGR  &=  ~( ADC_CFGR_DMACFG );
	adc->CFGR  &=  ~( ADC_CFGR_DMAEN );

	adc->CR  |=  ( ADC_CR_ADSTART ); //Continue conversion

}


/** Public Functions */
