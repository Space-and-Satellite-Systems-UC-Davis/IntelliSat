/**
 * @file flash_spi.c
 *
 * @brief Bit-banged single-line SPI (mode 0) to the W25Q128JV FLASH.
 *
 * @author Li, Chun Ho (lchli@ucdavis.edu)
 * @date 10/24/2026
 */

#include "flash_spi.h"
#include <GPIO/gpio.h>

// Same pins the QUADSPI peripheral used. IO0 is the FLASH's data in, IO1 its
// data out.
#if OP_REV == 2 || OP_REV == 3
#define FLASH_MOSI GPIOE,12
#define FLASH_MISO GPIOE,13
#elif OP_REV == 1
#define FLASH_MOSI GPIOB,1
#define FLASH_MISO GPIOB,0
#endif
#define FLASH_SCK  GPIOE,10
#define FLASH_CS   GPIOE,11
#define FLASH_WP   GPIOE,14
#define FLASH_HOLD GPIOE,15

void flashSpi_init() {
	// Set the levels before switching to output, so chip select never glitches
	// low. Mode 0 idles the clock low.
	gpio_high(FLASH_CS);
	gpio_high(FLASH_WP);
	gpio_high(FLASH_HOLD);
	gpio_low(FLASH_SCK);

	gpio_mode(FLASH_CS,   GPIO_MODER_Output, GPIO_OTYPER_PUSH_PULL, GPIO_OSPEEDR_MED, GPIO_PUPDR_NO_PULL);
	gpio_mode(FLASH_SCK,  GPIO_MODER_Output, GPIO_OTYPER_PUSH_PULL, GPIO_OSPEEDR_MED, GPIO_PUPDR_NO_PULL);
	gpio_mode(FLASH_MOSI, GPIO_MODER_Output, GPIO_OTYPER_PUSH_PULL, GPIO_OSPEEDR_MED, GPIO_PUPDR_NO_PULL);
	gpio_mode(FLASH_WP,   GPIO_MODER_Output, GPIO_OTYPER_PUSH_PULL, GPIO_OSPEEDR_MED, GPIO_PUPDR_NO_PULL);
	gpio_mode(FLASH_HOLD, GPIO_MODER_Output, GPIO_OTYPER_PUSH_PULL, GPIO_OSPEEDR_MED, GPIO_PUPDR_NO_PULL);
	gpio_mode(FLASH_MISO, GPIO_MODER_Input,  GPIO_OTYPER_PUSH_PULL, GPIO_OSPEEDR_MED, GPIO_PUPDR_PULL_UP);
}

static uint8_t transferByte(uint8_t out) {
	uint8_t in = 0;
	for (int bit = 7; bit >= 0; bit--) {
		gpio_set(FLASH_MOSI, (out >> bit) & 1);
		// The chip samples MOSI on the rising edge and changes MISO on the
		// falling one, so MISO is stable while the clock is high.
		gpio_high(FLASH_SCK);
		in = (in << 1) | gpio_read(FLASH_MISO);
		gpio_low(FLASH_SCK);
	}
	return in;
}

void flashSpi_command(uint8_t instruction, bool hasAddress, uint32_t address,
                      const uint8_t *tx, uint8_t *rx, uint32_t len) {
	gpio_low(FLASH_CS);
	transferByte(instruction);
	if (hasAddress) {
		transferByte(address >> 16);
		transferByte(address >> 8);
		transferByte(address);
	}
	for (uint32_t i = 0; i < len; i++) {
		uint8_t in = transferByte(tx ? tx[i] : 0xFF);
		if (rx) {
			rx[i] = in;
		}
	}
	gpio_high(FLASH_CS);
}
