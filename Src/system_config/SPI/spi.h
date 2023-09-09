/*
 * spi.h
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
#define SPI2_CS			  GPIOB,12
// allows spi communication to ignore the message received during a transmission
#define NO_RECEPTION_BYTE 0xA1

// `spi` is the SPI_TypeDef (SPI1 , SPI2, SPI3)
#define spi_enable(spi)  spi->CR1 |= SPI_CR1_SPE
void spi_disable(SPI_TypeDef *spi, GPIO_TypeDef *cs_port, int cs_pin);

void spi_start_communication(GPIO_TypeDef *cs_port, int cs_pin);
void spi_stop_communication(GPIO_TypeDef *cs_port, int cs_pin);

void spi1_config();
void spi2_config();

bool spi1_transmit_recieve(uint8_t trasnmission, uint8_t *reception);
bool spi2_transmit_recieve(uint8_t* transmission, uint8_t *reception, uint16_t size);

#endif
