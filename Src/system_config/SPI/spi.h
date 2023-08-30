/*
 * spi.h
 *
 *  - August 28, 2023 (Creation)
 *      Author	: Darsh
 *      Log		: ...
 */

#ifndef REALOP1_SPI_H_
#define REALOP1_SPI_H_

#include "stm32l476xx.h"
#include "../GPIO/gpio.h"

#define spi1_enable()  SPI11->SR |= SPI1_SR_SPE

void spi1_disable();
void spi1_config();
uint8_t spi1_transmit_recieve(uint8_t data);

#endif
