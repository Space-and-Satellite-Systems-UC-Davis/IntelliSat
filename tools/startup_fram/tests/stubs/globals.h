#pragma once
#include <stdbool.h>
#include <stdint.h>

/*
 * Host replacement for Src/inc/globals.h, which pulls the full CMSIS device
 * header. Only what Src/system_config/SPI/spi.h and the FRAM driver header need
 * to parse is declared here.
 */

#define OP_REV 3
#define UNUSED_GPIO GPIOG, 1

void delay_ms(uint64_t ms);
uint64_t getSysTime();
