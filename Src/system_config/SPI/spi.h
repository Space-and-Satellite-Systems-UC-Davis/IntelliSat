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
#include "../GPIO/gpio.h"

// predefined macros for cs_port,cs_pin (useful for later spi functions)
#define SPI1_CS           GPIOE,12
#define SPI1_CS_PUPD
#define SPI2_CS			  GPIOB,12
#define SPI2_CS_PUPD	  GPIO_PUPDR_PUPD12_0
#define SPI3_CS
#define SPI1_CS_PUPD

#define spi_enable(spi) spi->CR1 |= SPI_CR1_SPE
void spi_disable(SPI_TypeDef *spi, GPIO_TypeDef *cs_port, int cs_pin);

void spi1_config();
void spi2_config();
void spi3_config();

void spi_start_communication(GPIO_TypeDef *cs_port, int cs_pin);
void spi_stop_communication(GPIO_TypeDef *cs_port, int cs_pin);

bool spi1_transmit_recieve(uint8_t* trasnmission, uint8_t *reception, uint16_t size);
bool spi2_transmit_recieve(uint8_t* transmission, uint8_t *reception, uint16_t size);
bool spi3_transmit_recieve(uint8_t* transmission, uint8_t *reception, uint16_t size);

#endif
