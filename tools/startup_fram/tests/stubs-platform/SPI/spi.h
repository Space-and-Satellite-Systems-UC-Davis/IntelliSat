#pragma once

#include <stdbool.h>

#include "stm32l476xx.h"

/*
 * Host replacement for Src/system_config/SPI/spi.h: the provisioning entry only
 * needs the bus configuration call and the SPI1 chip-select pin owned by the FRAM
 * driver header.
 */
#define SPI1_CS GPIOA, 4

void spi_config(SPI_TypeDef *spi);
