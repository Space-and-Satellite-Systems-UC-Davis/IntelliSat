#include "adc_sun_sensors.h"

PANEL* panel0;
PANEL* panel1;
PANEL* panel2;
PANEL* panel3;
PANEL* panel4;
PANEL* panel5;

void adc_initializePanelConstants() {
    panel0->adcDiode0 = ADC3;
    panel0->adcDiode1 = ADC3;
    panel0->diode0Channel = 6;
    panel0->diode1Channel = 7;

    panel1->adcDiode0 = ADC3;
    panel1->adcDiode1 = ADC2;
    panel1->diode0Channel = 12;
    panel1->diode1Channel = 5;

    panel2->adcDiode0 = ADC2;
    panel2->adcDiode1 = ADC2;
    panel2->diode0Channel = 6;
    panel2->diode1Channel = 7;

    panel3->adcDiode0 = ADC3;
    panel3->adcDiode1 = ADC3;
    panel3->diode0Channel = 10;
    panel3->diode1Channel = 9;


    panel4->adcDiode0 = ADC2;
    panel4->adcDiode1 = ADC2;
    panel4->diode0Channel = 1;
    panel4->diode1Channel = 2;

    panel5->adcDiode0 = ADC2;
    panel5->adcDiode1 = ADC2;
    panel5->diode0Channel = 3;
    panel5->diode1Channel = 4;
}

bool adc1_isCalibrationDone() { return (ADC1->CR & ADC_CR_ADCAL) != 0; }
bool adc2_isCalibrationDone() { return (ADC2->CR & ADC_CR_ADCAL) != 0; }
bool adc3_isCalibrationDone() { return (ADC3->CR & ADC_CR_ADCAL) != 0; }

bool adc_isEOCDownADC1() { return !(ADC1->ISR & ADC_ISR_EOC); }
bool adc_isEOSDownADC1() { return !(ADC1->ISR & ADC_ISR_EOS); }

bool adc_isEOCDownADC2() { return !(ADC2->ISR & ADC_ISR_EOC); }
bool adc_isEOSDownADC2() { return !(ADC2->ISR & ADC_ISR_EOS); }

bool adc_isEOCDownADC3() { return !(ADC3->ISR & ADC_ISR_EOC); }
bool adc_isEOSDownADC3() { return !(ADC3->ISR & ADC_ISR_EOS); }

bool adc_isADRDYNotResetADC1() { return (ADC1->ISR & ADC_ISR_ADRDY) == 0; }
bool adc_isADRDYNotResetADC2() { return (ADC2->ISR & ADC_ISR_ADRDY) == 0; }
bool adc_isADRDYNotResetADC3() { return (ADC3->ISR & ADC_ISR_ADRDY) == 0; }



void ADC1_gpio_init() {
    // Currently not being used
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

    GPIOA->ASCR |= GPIO_ASCR_ASC0 | GPIO_ASCR_ASC1 | GPIO_ASCR_ASC2;
    GPIOC->ASCR |= GPIO_ASCR_ASC0 | GPIO_ASCR_ASC1 | GPIO_ASCR_ASC2 | GPIO_ASCR_ASC3; 

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

uint8_t adc_calibrateADC(ADC_TypeDef* adc) {
    adc->CR &= ~ADC_CR_DEEPPWD;//makes sure ADC isn't in deep power down mode
	adc->CR |= ADC_CR_ADVREGEN; //enables ADC voltage regulator
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

void adc_init() {
    adc_initializePanelConstants();
    ADC1_init();
    ADC2_init();
    ADC3_init();
}

void adcx_initCommon(ADC_TypeDef* adc) {
    RCC->APB2ENR |= RCC_AHB2ENR_ADCEN;
    adc_calibrateADC(adc);
    RCC->CCIPR &= ~RCC_CCIPR_ADCSEL; //enables peripheral clock
	RCC->CCIPR |= RCC_CCIPR_ADCSEL_SYSCLK; //Sets ADC clock to system clock
    ADC123_COMMON->CCR |= (ADC_CCR_VBATEN);
    adc->SQR1 &= ~( ADC_SQR1_L ); //Sets number of channels in the sequence of 1
	adc->SQR1 &= ~(ADC_SQR1_SQ1); //Resets the sequence
}

void ADC1_init() {
    ADC1_gpio_init();
    adcx_initCommon(ADC1);
    //Not being set because need to know what pins being used and ADC1 is currently not being used

}

void ADC2_init() {
    ADC2_gpio_init();
    adcx_initCommon(ADC2);
    ADC2->SMPR1 = 0; 

    //Set sample rate to slowest rate (640.5) per conversion 
    ADC2->SMPR1 |=  ADC_SMPR1_SMP1 | ADC_SMPR1_SMP2  | ADC_SMPR1_SMP3 | ADC_SMPR1_SMP4 | ADC_SMPR1_SMP5 | ADC_SMPR1_SMP6 | ADC_SMPR1_SMP7;
    adc_enable(ADC2);
}

void ADC3_init() {
    ADC3_gpio_init();
    adcx_initCommon(ADC3);

    //Set sample rate to slowest rate (640.5) per conversion 
    ADC3->SMPR1 = 0; 
    ADC3->SMPR1 |=  ADC_SMPR1_SMP6 | ADC_SMPR1_SMP7  | ADC_SMPR1_SMP9;
    ADC3->SMPR2 = 0;
    ADC3->SMPR2 |= ADC_SMPR2_SMP10 | ADC_SMPR2_SMP12; 

    adc_enable(ADC3);
}

void adc_enable(ADC_TypeDef* adc){
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


uint16_t adc_readVoltageFromDiode(PANEL* panel, int diodeNumber) {
    ADC_TypeDef* adc = NULL;
    int channel = -1;

    if (diodeNumber == DIODE0) {
        adc = panel->adcDiode0;
        channel = panel->diode0Channel;
    } else {
        adc = panel->adcDiode1;
        channel = panel->diode1Channel;
    }


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