/*
 * i2c.c
 *
 *  - Apr 29, 2023 (Creation)
 *      Author       : Tim S.
 *      Contributors : nithinsenthil , Raphael
 *      Log          : Created the I2C bitbang functions
 */

#include "i2c.h"

void softi2c_lineMode(GPIO_TypeDef * port, int pin, bool deassert_line) {
	if (deassert_line) {
		port->MODER &= ~(GPIO_MODER_Analog << (pin*GPIO_MODER_BITS_SPAN));
	} else {
		port->MODER = (port->MODER & ~(GPIO_MODER_Analog << (pin*GPIO_MODER_BITS_SPAN))) | (GPIO_OTYPER_OPEN_DRAIN << pin*GPIO_MODER_BITS_SPAN);
	}
}

void softi2c_init(GPIO_TypeDef * scl_port, int scl_pin, GPIO_TypeDef * sda_port, int sda_pin) {
	scl_port->OTYPER = (scl_port->OTYPER & ~(GPIO_OTYPER_OPEN_DRAIN << scl_pin)) | (GPIO_OTYPER_PUSH_PULL << scl_pin); // push pull
	sda_port->OTYPER = (sda_port->OTYPER & ~(GPIO_OTYPER_OPEN_DRAIN << sda_pin)) | (GPIO_OTYPER_PUSH_PULL << sda_pin);
	scl_port->OSPEEDR = (scl_port->OSPEEDR & ~(GPIO_OSPEEDR_VERY_HIGH << (scl_pin*GPIO_OSPEEDR_BITS_SPAN))) | (GPIO_OSPEEDR_VERY_HIGH << scl_pin*GPIO_OSPEEDR_BITS_SPAN); // very high speed
	sda_port->OSPEEDR = (sda_port->OSPEEDR & ~(GPIO_OSPEEDR_VERY_HIGH << (sda_pin*GPIO_OSPEEDR_BITS_SPAN))) | (GPIO_OSPEEDR_VERY_HIGH << sda_pin*GPIO_OSPEEDR_BITS_SPAN);
	scl_port->BSRR = GPIO_BSRR_BR_ODX_RESET << (scl_pin + GPIO_BSRR_BR0_Pos); // drive low when asserted
	sda_port->BSRR = GPIO_BSRR_BR_ODX_RESET << (sda_pin + GPIO_BSRR_BR0_Pos);
	softi2c_lineMode(scl_port, scl_pin, 1);
	softi2c_lineMode(sda_port, sda_pin, 1);
}

void softi2c_delay() {
	nop(30);
}

void softi2c_sigStart(GPIO_TypeDef * scl_port, int scl_pin, GPIO_TypeDef * sda_port, int sda_pin) {
	softi2c_lineMode(scl_port, scl_pin, 1);
	softi2c_lineMode(sda_port, sda_pin, 1);
	softi2c_delay(); softi2c_delay();
	softi2c_lineMode(sda_port, sda_pin, 0);
	softi2c_delay(); softi2c_delay();
	softi2c_lineMode(scl_port, scl_pin, 0);
	softi2c_delay();
}

void softi2c_sigRepeatedStart(GPIO_TypeDef * scl_port, int scl_pin, GPIO_TypeDef * sda_port, int sda_pin) {
	softi2c_lineMode(scl_port, scl_pin, 0);
	softi2c_lineMode(sda_port, sda_pin, 1);
	softi2c_delay();
	softi2c_lineMode(scl_port, scl_pin, 1);
	softi2c_delay(); softi2c_delay();
	softi2c_lineMode(sda_port, sda_pin, 0);
	softi2c_delay(); softi2c_delay();
	softi2c_lineMode(scl_port, scl_pin, 0);
	softi2c_delay();
}

void softi2c_sigStop(GPIO_TypeDef * scl_port, int scl_pin, GPIO_TypeDef * sda_port, int sda_pin) {
	softi2c_lineMode(scl_port, scl_pin, 0);
	softi2c_lineMode(sda_port, sda_pin, 0);
	softi2c_delay(); softi2c_delay();
	softi2c_lineMode(scl_port, scl_pin, 1);
	softi2c_delay(); softi2c_delay();
	softi2c_lineMode(sda_port, sda_pin, 1);
	softi2c_delay();
}

void softi2c_send8(GPIO_TypeDef * scl_port, int scl_pin, GPIO_TypeDef * sda_port, int sda_pin, int data8) {
	for (int i = 0; i < 8; i++) {
		softi2c_lineMode(scl_port, scl_pin, 0);
		softi2c_delay();
		softi2c_lineMode(sda_port, sda_pin, (data8 << i) & MSB_8BIT_Msk);
		softi2c_delay();
		softi2c_lineMode(scl_port, scl_pin, 1);
		softi2c_delay(); softi2c_delay();
	}
}

int softi2c_read8(GPIO_TypeDef * scl_port, int scl_pin, GPIO_TypeDef * sda_port, int sda_pin) {
	int data = 0;
	softi2c_lineMode(sda_port, sda_pin, 1);
	for (int i = 0; i < 8; i++) {
		softi2c_lineMode(scl_port, scl_pin, 0);
		softi2c_delay();
		data |= gpio_read(sda_port, sda_pin) << (_8th_BIT_OFFSET-i);
		softi2c_delay();
		softi2c_lineMode(scl_port, scl_pin, 1);
		softi2c_delay(); softi2c_delay();
	}
	return data;
}

void softi2c_sendNack(GPIO_TypeDef * scl_port, int scl_pin, GPIO_TypeDef * sda_port, int sda_pin) {
	softi2c_lineMode(scl_port, scl_pin, 0);
	softi2c_delay();
	softi2c_lineMode(sda_port, sda_pin, 1);
	softi2c_delay();
	softi2c_lineMode(scl_port, scl_pin, 1);
	softi2c_delay();
}

int softi2c_readNack(GPIO_TypeDef * scl_port, int scl_pin, GPIO_TypeDef * sda_port, int sda_pin) {
	softi2c_lineMode(scl_port, scl_pin, 0);
	softi2c_delay();
	softi2c_lineMode(sda_port, sda_pin, 1);
	softi2c_delay();
	softi2c_lineMode(scl_port, scl_pin, 1);
	softi2c_delay();
	int nack = gpio_read(sda_port, sda_pin);
	softi2c_delay();
	return nack;
}

int softi2c_writeReg(GPIO_TypeDef * scl_port, int scl_pin, GPIO_TypeDef * sda_port, int sda_pin, int device_addr, int reg_addr, int data) {
	int nack = 0;
	softi2c_sigStart(scl_port, scl_pin, sda_port, sda_pin);
	softi2c_send8(scl_port, scl_pin, sda_port, sda_pin, device_addr << 1 | 0);
	nack += softi2c_readNack(scl_port, scl_pin, sda_port, sda_pin);
	softi2c_send8(scl_port, scl_pin, sda_port, sda_pin, reg_addr);
	nack += softi2c_readNack(scl_port, scl_pin, sda_port, sda_pin);
	softi2c_send8(scl_port, scl_pin, sda_port, sda_pin, data);
	nack += softi2c_readNack(scl_port, scl_pin, sda_port, sda_pin);
	softi2c_sigStop(scl_port, scl_pin, sda_port, sda_pin);
	return nack;
}

int softi2c_readReg(GPIO_TypeDef * scl_port, int scl_pin, GPIO_TypeDef * sda_port, int sda_pin, int device_addr, int reg_addr) {
	int nack = 0;
	softi2c_sigStart(scl_port, scl_pin, sda_port, sda_pin);
	softi2c_send8(scl_port, scl_pin, sda_port, sda_pin, device_addr << 1 | 0);
	nack += softi2c_readNack(scl_port, scl_pin, sda_port, sda_pin);
	softi2c_send8(scl_port, scl_pin, sda_port, sda_pin, reg_addr);
	nack += softi2c_readNack(scl_port, scl_pin, sda_port, sda_pin);
	softi2c_sigRepeatedStart(scl_port, scl_pin, sda_port, sda_pin);
	softi2c_send8(scl_port, scl_pin, sda_port, sda_pin, device_addr << 1 | 1);
	nack += softi2c_readNack(scl_port, scl_pin, sda_port, sda_pin);
	int data = softi2c_read8(scl_port, scl_pin, sda_port, sda_pin);
	softi2c_sendNack(scl_port, scl_pin, sda_port, sda_pin);
	softi2c_sigStop(scl_port, scl_pin, sda_port, sda_pin);
	return data;
}

bool softi2c_probe(GPIO_TypeDef * scl_port, int scl_pin, GPIO_TypeDef * sda_port, int sda_pin, int device_addr) {
	int nack = 0;
	softi2c_sigStart(scl_port, scl_pin, sda_port, sda_pin);
	softi2c_send8(scl_port, scl_pin, sda_port, sda_pin, device_addr << 1 | 1);
	nack += softi2c_readNack(scl_port, scl_pin, sda_port, sda_pin);
	softi2c_sigStop(scl_port, scl_pin, sda_port, sda_pin);
	if (nack) {
		return 0;
	} else {
		return 1;
	}
}

int16_t softi2c_readRegHighLow(GPIO_TypeDef * scl_port, int scl_pin, GPIO_TypeDef * sda_port, int sda_pin, int device_addr, int high_reg_addr, int low_reg_addr) {
	return softi2c_readReg(scl_port, scl_pin, sda_port, sda_pin, device_addr, high_reg_addr) << 8 | softi2c_readReg(scl_port, scl_pin, sda_port, sda_pin, device_addr, low_reg_addr);
}
