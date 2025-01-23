/*
 * spi.h
 *
 *	- October 29, 2023
 *		Author	: Darsh
 *		Log		: Generic spi functions.
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
#include <globals.h>
#include <GPIO/gpio.h>

// predefined macros for cs_port,cs_pin (useful for later spi functions)
#if OP_REV == 1

#define SPI1_CS_GPIO	  UNUSED_GPIO_GPIO
#define SPI1_CS_PIN       UNUSED_GPIO_PIN
#define SPI2_CS_GPIO	  GPIOB
#define SPI2_CS_PIN       12
#define SPI3_CS_GPIO	  UNUSED_GPIO_GPIO
#define SPI3_CS_PIN		  UNUSED_GPIO_PIN

#elif OP_REV == 2 || OP_REV == 3

#define SPI1_CS_GPIO      GPIOA 
#define SPI1_CS_PIN       4
#define SPI2_CS_GPIO      UNUSED_GPIO_GPIO
#define SPI2_CS_PIN	      UNUSED_GPIO_PIN 
#define SPI3_CS_GPIO	  GPIOG
#define SPI3_CS_PIN       15

#endif

#define SPI_DUMMY_BYTE 0xAA

/**************************** SPI INITIALIZATIONS ****************************/

void spi_enable(SPI_TypeDef *spi);

/**
 * Stops an SPI peripheral
*/
void spi_disable(SPI_TypeDef *spi, GPIO_TypeDef *cs_port, int cs_pin);

/**
 * Configures an SPI line to be able to transmit_recieve() later
*/
void spi_config(SPI_TypeDef *spi);

/***************************** SPI COMMUNICATION *****************************/

/**
 * Starts SPI communication for a peripheral by setting the CS pin back to LOW
 * 
 * @param cs_port The GPIO port of the CS pin
 * @param cs_pin  The GPIO pin number of the CS pin
 * 
 * @return None. Toggles the pin to LOW in the background
*/
void spi_startCommunication(GPIO_TypeDef *cs_port, int cs_pin);
/**
 * Stops SPI communication for a peripheral by setting the CS pin back to HIGH
 * 
 * @param cs_port The GPIO port of the CS pin
 * @param cs_pin  The GPIO pin number of the CS pin
 * 
 * @return None. Toggles the pin to HIGH in the background
*/
void spi_stopCommunication(GPIO_TypeDef *cs_port, int cs_pin);

/**
 * Transmits and Recieves messages over an spi line
 * Assumes that communication has already been started using spi_startCommunicationunication()
 * 
 * NOTE: 1 call of this function will transmit out `size` bytes of data from the 
 * transmission buffer. In parallel, it will store in the `size` bytes that were present
 * on the MISO line during that transmission phase.
 * If you only want to transmit, (recieve nothing at all), set `reception = NULL`
 * If you only want to recieve, (not transmit anything at all), set `transmission = NULL`
 * 
 * @param spi          The SPI line on which the communication is happening
 * @param transmission The buffer which contains the bytes to be sent (must have a length of `size` unless it's `NULL`)
 * @param reception    The buffer where the bytes read will be stored (must have a length of `size` unless it's `NULL`)
 * @param size         The number of bytes involved in the instruction
 * @param dma		   Don't worry about this for now
 * 
 * @returns Boolean to indicate if the communication was successful or not
*/
bool spi_transmitReceive(SPI_TypeDef* spi, uint8_t* transmission, uint8_t *reception, uint16_t size, bool dma);

#endif	// REALOP1_SPI_H_
