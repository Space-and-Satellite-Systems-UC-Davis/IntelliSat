/*
 * clock_nvic_config.c
 *
 *	- August 28, 2023
 *		Author	: Darsh
 *		Log		: clocks for SPI-1
 *
 *	- May 14-16, 2023
 * 		Author       : Raphael, Darsh, Parteek
 *      Contributors : nithilsenthil, Huey, Raymond, Kevin
 *      Log          : Enabled interrupts for the buttons
 *
 *  - May 22, 2023
 *  	Author       : Darsh
 *  	Contributors : Parteek
 *  	Log          : Turned on LSI, LSE, wrote rtc_clock_config()
 *
 *	- May 11, 2023
 *		Author : Darsh
 *		Log    : Replacing hardcoded values with placeholders from
 *				 stm32l476xx.h, as well as commenting
 *
 *  - Apr 29, 2023 (Creation)
 *      Author       : Tim S.
 *      Contributors : nithinsenthil , Raphael
 *      Log          : All necessary clock registers configured
 */

#include "clock_nvic_config.h"

// initializing global counters and trackers
int core_MHz = 0;
int heartbeat_counter = 0;
int ag_counter = 0;


#define LSI 'i'
#define LSE 'l'
#define HSI 'h'
#define HSE 'x'

/**
 * Selects and Oscillator for the RTC to use, and enables the RTC's Clock Source.
 * NOTE : The selected Oscillator must be turned on beforehand.
 *
 * @param clock_source   An character to specify which oscillator the RTC uses. There are predefined options in clock_nvic_config.h
 * @param forced_config  Setting this to 'false' results in the configuration not taking place in case the RTC is pre-initialized
 * @returns None
 *
 */
void rtc_clock_config(char clock_source, int forced_config) {
	// do nothing if clock is already configured
	if ((RTC->ISR & RTC_ISR_INITS) && !forced_config) {
		return;
	}

	backup_domain_control_enable();

	// store the current clock configuration, in case of bad input
	uint32_t temp = RCC->BDCR | RCC_BDCR_RTCSEL;

	// reset the clock
	RCC->BDCR &= ~RCC_BDCR_RTCSEL;

	// Select the RTC clock source
	switch (clock_source) {
		case LSE:
			RCC->BDCR |= RCC_BDCR_RTCSEL_0;
			break;
		case LSI:
			RCC->BDCR |= RCC_BDCR_RTCSEL_1;
			break;
		case HSE:
			RCC->BDCR |= RCC_BDCR_RTCSEL_Msk;
			break;
		default:
			RCC->BDCR |= temp;	// restore the original configuration
			break;
	}

	// Enable the RTC Clock
	RCC->BDCR |= RCC_BDCR_RTCEN;

	backup_domain_control_disable();
}

/**
 * Initializes the clocks of the microcontroller.
 *
 * This function sets up the various clock sources and frequencies for the microcontroller.
 * It also reads the unique ID of the microcontroller and stores it in an array.
 *
 * @param None
 * @returns None
 */
void init_clocks() {
	// enable clock to different peripherals
	RCC->AHB2ENR =
		  RCC_AHB2ENR_GPIOAEN	// enable GPIO Port A
		| RCC_AHB2ENR_GPIOBEN	// enable GPIO Port B
		| RCC_AHB2ENR_GPIOCEN	// enable GPIO Port C
		| RCC_AHB2ENR_GPIODEN	// enable GPIO Port D
		| RCC_AHB2ENR_GPIOEEN	// enable GPIO Port E
		| RCC_AHB2ENR_GPIOFEN	// enable GPIO Port F
		| RCC_AHB2ENR_GPIOGEN	// enable GPIO Port G
		| RCC_AHB2ENR_GPIOHEN	// enable GPIO Port H
		| RCC_AHB2ENR_OTGFSEN	// enable OTG full speed
		| RCC_AHB2ENR_ADCEN		// enable ADC
		| RCC_AHB2ENR_RNGEN;	// enable Random Number Generator
	RCC->AHB3ENR = RCC_AHB3ENR_QSPIEN;	// enable clock to QSPI
	// enable clock to different perihpherals and protocol hardware
	RCC->APB1ENR1 =
		  RCC_APB1ENR1_PWREN 		// enable Power Control
		| RCC_APB1ENR1_I2C2EN 		// enable I2C2
		| RCC_APB1ENR1_SPI3EN 		// enable SPI3
		| RCC_APB1ENR1_SPI2EN 		// enable SPI2
		| RCC_APB1ENR1_USART3EN;	// enable USART3

	// enable Syscfg
	RCC->APB2ENR =
		  RCC_APB2ENR_SYSCFGEN		// enable Syscfg
		| RCC_APB2ENR_SPI1EN;		// enable SPI1

	// configure Flash Memory
	FLASH->ACR =
		  FLASH_ACR_DCEN			// Flash data cache enable
		| FLASH_ACR_ICEN			// Flash instruction cache enable
		| FLASH_ACR_PRFTEN			// Flash prefetch enable
		| FLASH_ACR_LATENCY_4WS;	// 4 HCLCK periods of latency in Flash access time

	// enable internal oscillators
	RCC->CR |= RCC_CR_HSION; 				// enable HSI
	RCC->CSR |= RCC_CSR_LSION;				// Turn on the LSI Oscillator
	while (!(RCC->CSR & RCC_CSR_LSIRDY));	// wait for the LSI Oscillator to stabilize
//	RCC->BDCR |= RCC_BDCR_LSEON;			// Turn on the LSE Oscillator
//	while (!(RCC->BDCR & RCC_BDCR_LSERDY));	// wait for the LSE Oscillator to stabilize

	// configure Phased Lock Loop
	RCC->PLLCFGR =
		  0 << RCC_PLLCFGR_PLLR_Pos 	// PLLR = 2
		| RCC_PLLCFGR_PLLREN 			// PLLCLK output enabled
		| 0 << RCC_PLLCFGR_PLLQ_Pos 	//
		| RCC_PLLCFGR_PLLQEN			// PLLQEN (PLL48M1CLK) output enabled
		| 0 << RCC_PLLCFGR_PLLP_Pos 	// PLLP default
		| 0 << RCC_PLLCFGR_PLLPEN_Pos 	// PLLSAI3 not enabled
		| (10 << RCC_PLLCFGR_PLLN_Pos) 	// PLLN = 10
		| 0 << RCC_PLLCFGR_PLLM_Pos 	// PLLM = 1
		| RCC_PLLCFGR_PLLSRC_HSI; 		// HSI16 as input clock to PLLs
	RCC->PLLSAI1CFGR =
		  RCC_PLLSAI1CFGR_PLLSAI1R_0		 	// PLLADC1CLK division factor set to 4
		| RCC_PLLSAI1CFGR_PLLSAI1REN 			// PLLADC1CLK output enable
		| RCC_PLLSAI1CFGR_PLLSAI1Q_0 			// PLL48M2CLK division factor set to 4
		| RCC_PLLSAI1CFGR_PLLSAI1QEN 			// PLL48M2CLK output enable
		| 0 << RCC_PLLSAI1CFGR_PLLSAI1P_Pos		// PLLSAI1CLKC division factor set to 7
		| 0 << RCC_PLLSAI1CFGR_PLLSAI1PEN_Pos 	// PLLSAI1CLK output disabled
		| 12 << RCC_PLLSAI1CFGR_PLLSAI1N_Pos; 	// VCO multiplication factor set to 12
	// enable PLL and PLLSAI1
	RCC->CR |= RCC_CR_PLLON | RCC_CR_PLLSAI1ON;
	// wait for PLL lock
	while (!(RCC->CR & RCC_CR_PLLRDY));
	// wait for PLLSAI1 lock
	while (!(RCC->CR & RCC_CR_PLLSAI1RDY));
	// system clock to PLL
	RCC->CFGR = RCC_CFGR_SW;

	rtc_clock_config(LSI, 0);

	core_MHz = 80;
}

/**
 * Initializes the Nested Vector Interrupt Controller (NVIC) for
 * 		- Systick Timer (1ms)
 * 		- GPIO Pins 10-15
 * 			- Buttons 0 & 1
 *
 * @param None
 * @returns None
 */
void init_nvic() {
	__disable_irq();

	// configure for 1 ms period
	SysTick->LOAD = (core_MHz / 8) * 1000;
	// use AHB/8 as input clock, and enable counter interrupt
	SysTick->CTRL = 0x3;
	NVIC_EnableIRQ(SysTick_IRQn);

	/*
	 * Pins PB10 and PB11 are connected to buttons and trigger an interrupt
	 * Thus the interrrupt must be configured.
	 */

	// Set a high priority for pin 10-15 interrupts, and enable the interrupts
	NVIC_SetPriority(EXTI15_10_IRQn, 1);
	NVIC_EnableIRQ(EXTI15_10_IRQn);

	__enable_irq();
}


/**
 * Enables writing access to registers powered by the Backup Domain
 *    Key registers include RCC's BDRC, and several key RTC registers
 *
 * @param None
 *
 * @returns None
 */
void backup_domain_control_enable() {
	PWR->CR1 |= PWR_CR1_DBP;
}

/**
 * Disables writing access to registers powered by the Backup Domain
 *    Key registers include RCC's BDRC, and several key RTC registers
 *
 * @param None
 *
 * @returns None
 */
void backup_domain_control_disable() {
	PWR->CR1 &= ~PWR_CR1_DBP;
}


// ----------------------------------------------------------------------------

void nop(int nop_loops) {
	for (int i = 0; i < nop_loops; i++) {__NOP();}
}
