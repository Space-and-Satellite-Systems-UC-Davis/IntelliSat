/*
 * spi.h
 *
 *	- September 22, 2023
 *		Author	: Darsh
 *		Log		: Included all stages of initializations in spi.h / spi.c
 *
 *  - August 28-31, 2023 (Creation)
 *      Author	: Darsh
 *      Log		: SPI 1 config, disable, transmitrecieve
 */

#ifndef REALOP1_SPI_H_
#define REALOP1_SPI_H_

#include "stm32l476xx.h"
#include "../../globals.h"
#include "../GPIO/gpio.h"

// predefined macros for cs_port,cs_pin (useful for later spi functions)
#define SPI1_CS           GPIOE,12
#define SPI1_CS_PUPD
#define SPI2_CS			  GPIOB,12
#define SPI2_CS_PUPD	  GPIO_PUPDR_PUPD12_0
#define SPI3_CS
#define SPI1_CS_PUPD

#define SPI_DUMMY_BYTE 0xAA

/**************************** SPI INITIALIZATIONS ****************************/

#define spi_enable(spi) spi->CR1 |= SPI_CR1_SPE

/**
 * Stops an SPI peripheral
 * 
*/
void spi_disable(SPI_TypeDef *spi, GPIO_TypeDef *cs_port, int cs_pin);

/**
 * Configures the SPI-1 Hardware to be able to transmit_recieve() later
*/
void spi1_config();
/**
 * Configures the SPI-2 Hardware to be able to transmit_recieve() later
*/
void spi2_config();
/**
 * Configures the SPI-1 Hardware to be able to transmit_recieve() later
*/
void spi3_config();

/***************************** SPI COMMUNICATION *****************************/

/**
 * Starts SPI communication for a peripheral by setting the CS pin back to LOW
 * 
 * @param cs_port The GPIO port of the CS pin
 * @param cs_pin  The GPIO pin number of the CS pin
 * 
 * @return None. Toggles the pin to LOW in the background
*/
void spi_start_communication(GPIO_TypeDef *cs_port, int cs_pin);
/**
 * Stops SPI communication for a peripheral by setting the CS pin back to HIGH
 * 
 * @param cs_port The GPIO port of the CS pin
 * @param cs_pin  The GPIO pin number of the CS pin
 * 
 * @return None. Toggles the pin to HIGH in the background
*/
void spi_stop_communication(GPIO_TypeDef *cs_port, int cs_pin);

/**
 * not implemented, but similar in concept to spi2_transmit_recieve()
*/
bool spi1_transmit_recieve(uint8_t* trasnmission, uint8_t *reception, uint16_t size);
/**
 * Transmits and Recieves messages over the SPI-2 bus.
 * Assumes that communication has already been started using spi_start_communication()
 * 
 * NOTE: 1 call of this function will transmit out `size` bytes of data from the 
 * transmission buffer. In parallel, it will store in the 16 bytes that were present 
 * on the MISO line during that transmission phase.
 * If you only want to transmit, (recieve nothing at all), set `reception = NULL`
 * If you only want to recieve, (not transmit anything at all), set `transmission = NULL`
 * 
 * @param transmission The buffer which contains the bytes to be sent (must have a length of `size` unless it's `NULL`)
 * @param reception    The buffer where the bytes read will be stored (must have a length of `size` unless it's `NULL`)
 * @param size         The number of bytes involved in the instruciton
 * 
 * @returns Boolean to indicate if the communication was successful or not
*/
bool spi2_transmit_recieve(uint8_t* transmission, uint8_t *reception, uint16_t size);
/**
 * not implemented, but similar in concept to spi2_transmit_recieve()
*/
bool spi3_transmit_recieve(uint8_t* transmission, uint8_t *reception, uint16_t size);

#endif	// REALOP1_SPI_H_
