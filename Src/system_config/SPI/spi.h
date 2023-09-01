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

#define SPI1_CS           GPIOE,12
#define NO_RECEPTION_BYTE 0xA1


#define spi1_enable()  SPI11->SR |= SPI1_SR_SPE
void spi1_disable();

#define spi1_start_communication() gpio_low(SPI1_CS)
#define spi1_end_communication()   gpio_high(SPI1_CS)

void spi1_config();
bool spi1_transmit_recieve(uint8_t trasnmission, uint8_t *reception);

#endif
