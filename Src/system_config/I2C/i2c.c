/*
 * i2c.c
 *
 *  - Apr 29, 2023 (Creation)
 *      Author       : Tim S.
 *      Contributors : nithinsenthil , Raphael
 *      Log          : Created the I2C bitbang functions
 */

#include "i2c.h"

/**
 * Sets the line mode of a GPIO pin for Software I2C communication.
 *
 * @param port The GPIO port to set the line mode for.
 * @param pin The pin number to set the line mode for.
 * @param deassert_line A boolean indicating whether to deassert the line or not.
 *                      If true, the line is deasserted. If false, the line is set to output mode.
 *
 * @returns None
 */
void softi2c_line_mode(GPIO_TypeDef * port, int pin, bool deassert_line) {
	if (deassert_line) {
		port->MODER &= ~(0x3 << (pin*2));
	} else {
		port->MODER = (port->MODER & ~(0x3 << (pin*2))) | (1 << pin*2);
	}
}

/**
 * Initializes the software implementation of the I2C protocol.
 *
 * @param scl_port The GPIO port for the SCL line.
 * @param scl_pin The GPIO pin for the SCL line.
 * @param sda_port The GPIO port for the SDA line.
 * @param sda_pin The GPIO pin for the SDA line.
 *
 * @returns None
 */
void init_softi2c(GPIO_TypeDef * scl_port, int scl_pin, GPIO_TypeDef * sda_port, int sda_pin) {
	scl_port->OTYPER = (scl_port->OTYPER & ~(1 << scl_pin)) | (0 << scl_pin); // push pull
	sda_port->OTYPER = (sda_port->OTYPER & ~(1 << sda_pin)) | (0 << sda_pin);
	scl_port->OSPEEDR = (scl_port->OSPEEDR & ~(0x3 << (scl_pin*2))) | (3 << scl_pin*2); // very high speed
	sda_port->OSPEEDR = (sda_port->OSPEEDR & ~(0x3 << (sda_pin*2))) | (3 << sda_pin*2);
	scl_port->BSRR = 1 << (scl_pin + 16); // drive low when asserted
	sda_port->BSRR = 1 << (sda_pin + 16);
	softi2c_line_mode(scl_port, scl_pin, 1);
	softi2c_line_mode(sda_port, sda_pin, 1);
}

/**
 * Delays the execution of the program for a short period of time to allow for proper timing in the Software I2C protocol.
 *
 * @returns None
 */
void softi2c_delay() {
	nop(30);
}

/**
 * Generates the start signal for a software-based I2C communication.
 *
 * @param scl_port The GPIO port for the SCL line.
 * @param scl_pin The GPIO pin for the SCL line.
 * @param sda_port The GPIO port for the SDA line.
 * @param sda_pin The GPIO pin for the SDA line.
 *
 * @returns None
 */
void softi2c_sig_start(GPIO_TypeDef * scl_port, int scl_pin, GPIO_TypeDef * sda_port, int sda_pin) {
	softi2c_line_mode(scl_port, scl_pin, 1);
	softi2c_line_mode(sda_port, sda_pin, 1);
	softi2c_delay(); softi2c_delay();
	softi2c_line_mode(sda_port, sda_pin, 0);
	softi2c_delay(); softi2c_delay();
	softi2c_line_mode(scl_port, scl_pin, 0);
	softi2c_delay();
}

/**
 * Generates a repeated start signal for the Software I2C protocol.
 *
 * @param scl_port The GPIO port for the SCL line.
 * @param scl_pin The GPIO pin for the SCL line.
 * @param sda_port The GPIO port for the SDA line.
 * @param sda_pin The GPIO pin for the SDA line.
 *
 * @returns None
 */
void softi2c_sig_repeated_start(GPIO_TypeDef * scl_port, int scl_pin, GPIO_TypeDef * sda_port, int sda_pin) {
	softi2c_line_mode(scl_port, scl_pin, 0);
	softi2c_line_mode(sda_port, sda_pin, 1);
	softi2c_delay();
	softi2c_line_mode(scl_port, scl_pin, 1);
	softi2c_delay(); softi2c_delay();
	softi2c_line_mode(sda_port, sda_pin, 0);
	softi2c_delay(); softi2c_delay();
	softi2c_line_mode(scl_port, scl_pin, 0);
	softi2c_delay();
}

/**
 * Sends a stop signal on the I2C bus using software implementation.
 *
 * @param scl_port The GPIO port of the SCL line.
 * @param scl_pin The GPIO pin of the SCL line.
 * @param sda_port The GPIO port of the SDA line.
 * @param sda_pin The GPIO pin of the SDA line.
 *
 * @returns None
 */
void softi2c_sig_stop(GPIO_TypeDef * scl_port, int scl_pin, GPIO_TypeDef * sda_port, int sda_pin) {
	softi2c_line_mode(scl_port, scl_pin, 0);
	softi2c_line_mode(sda_port, sda_pin, 0);
	softi2c_delay(); softi2c_delay();
	softi2c_line_mode(scl_port, scl_pin, 1);
	softi2c_delay(); softi2c_delay();
	softi2c_line_mode(sda_port, sda_pin, 1);
	softi2c_delay();
}

/**
 * Sends an 8-bit data over an I2C bus.
 *
 * @param scl_port The GPIO port of the SCL line.
 * @param scl_pin The GPIO pin of the SCL line.
 * @param sda_port The GPIO port of the SDA line.
 * @param sda_pin The GPIO pin of the SDA line.
 * @param data8 The 8-bit data to be sent.
 *
 * @returns None
 */
void softi2c_send8(GPIO_TypeDef * scl_port, int scl_pin, GPIO_TypeDef * sda_port, int sda_pin, int data8) {
	for (int i = 0; i < 8; i++) {
		softi2c_line_mode(scl_port, scl_pin, 0);
		softi2c_delay();
		softi2c_line_mode(sda_port, sda_pin, (data8 << i) & (1 << 7));
		softi2c_delay();
		softi2c_line_mode(scl_port, scl_pin, 1);
		softi2c_delay(); softi2c_delay();
	}
}

/**
 * Reads 8 bits of data from an I2C bus.
 *
 * @param scl_port The GPIO port of the SCL line.
 * @param scl_pin The GPIO pin of the SCL line.
 * @param sda_port The GPIO port of the SDA line.
 * @param sda_pin The GPIO pin of the SDA line.
 *
 * @returns The 8 bits of data read from the Software I2C bus.
 */
int softi2c_read8(GPIO_TypeDef * scl_port, int scl_pin, GPIO_TypeDef * sda_port, int sda_pin) {
	int data = 0;
	softi2c_line_mode(sda_port, sda_pin, 1);
	for (int i = 0; i < 8; i++) {
		softi2c_line_mode(scl_port, scl_pin, 0);
		softi2c_delay();
		data |= gpio_read(sda_port, sda_pin) << (7-i);
		softi2c_delay();
		softi2c_line_mode(scl_port, scl_pin, 1);
		softi2c_delay(); softi2c_delay();
	}
	return data;
}

/**
 * Sends a NACK (Not Acknowledged) signal over a software I2C bus.
 *
 * @param scl_port The GPIO port of the SCL line.
 * @param scl_pin The GPIO pin of the SCL line.
 * @param sda_port The GPIO port of the SDA line.
 * @param sda_pin The GPIO pin of the SDA line.
 *
 * @returns None
 */
void softi2c_send_nack(GPIO_TypeDef * scl_port, int scl_pin, GPIO_TypeDef * sda_port, int sda_pin) {
	softi2c_line_mode(scl_port, scl_pin, 0);
	softi2c_delay();
	softi2c_line_mode(sda_port, sda_pin, 1);
	softi2c_delay();
	softi2c_line_mode(scl_port, scl_pin, 1);
	softi2c_delay();
}

/**
 * Reads a single byte from an I2C device with a NACK signal.
 *
 * @param scl_port The GPIO port of the SCL line.
 * @param scl_pin The GPIO pin of the SCL line.
 * @param sda_port The GPIO port of the SDA line.
 * @param sda_pin The GPIO pin of the SDA line.
 *
 * @returns The byte read from the I2C device.
 */
int softi2c_read_nack(GPIO_TypeDef * scl_port, int scl_pin, GPIO_TypeDef * sda_port, int sda_pin) {
	softi2c_line_mode(scl_port, scl_pin, 0);
	softi2c_delay();
	softi2c_line_mode(sda_port, sda_pin, 1);
	softi2c_delay();
	softi2c_line_mode(scl_port, scl_pin, 1);
	softi2c_delay();
	int nack = gpio_read(sda_port, sda_pin);
	softi2c_delay();
	return nack;
}

/**
 * Writes data to a register of a device using software I2C.
 *
 * @param scl_port The GPIO port of the SCL pin.
 * @param scl_pin The pin number of the SCL pin.
 * @param sda_port The GPIO port of the SDA pin.
 * @param sda_pin The pin number of the SDA pin.
 * @param device_addr The address of the device to write to.
 * @param reg_addr The address of the register to write to.
 * @param data The data to write to the register.
 *
 * @returns The number of NACKs received during the write operation.
 */
int softi2c_write_reg(GPIO_TypeDef * scl_port, int scl_pin, GPIO_TypeDef * sda_port, int sda_pin, int device_addr, int reg_addr, int data) {
	int nack = 0;
	softi2c_sig_start(scl_port, scl_pin, sda_port, sda_pin);
	softi2c_send8(scl_port, scl_pin, sda_port, sda_pin, device_addr << 1 | 0);
	nack += softi2c_read_nack(scl_port, scl_pin, sda_port, sda_pin);
	softi2c_send8(scl_port, scl_pin, sda_port, sda_pin, reg_addr);
	nack += softi2c_read_nack(scl_port, scl_pin, sda_port, sda_pin);
	softi2c_send8(scl_port, scl_pin, sda_port, sda_pin, data);
	nack += softi2c_read_nack(scl_port, scl_pin, sda_port, sda_pin);
	softi2c_sig_stop(scl_port, scl_pin, sda_port, sda_pin);
	return nack;
}

/**
 * Reads a register from a device using software I2C.
 *
 * @param scl_port The GPIO port of the SCL pin.
 * @param scl_pin The GPIO pin number of the SCL pin.
 * @param sda_port The GPIO port of the SDA pin.
 * @param sda_pin The GPIO pin number of the SDA pin.
 * @param device_addr The address of the device to read from.
 * @param reg_addr The address of the register to read.
 *
 * @returns The value of the register read from the device.
 */
int softi2c_read_reg(GPIO_TypeDef * scl_port, int scl_pin, GPIO_TypeDef * sda_port, int sda_pin, int device_addr, int reg_addr) {
	int nack = 0;
	softi2c_sig_start(scl_port, scl_pin, sda_port, sda_pin);
	softi2c_send8(scl_port, scl_pin, sda_port, sda_pin, device_addr << 1 | 0);
	nack += softi2c_read_nack(scl_port, scl_pin, sda_port, sda_pin);
	softi2c_send8(scl_port, scl_pin, sda_port, sda_pin, reg_addr);
	nack += softi2c_read_nack(scl_port, scl_pin, sda_port, sda_pin);
	softi2c_sig_repeated_start(scl_port, scl_pin, sda_port, sda_pin);
	softi2c_send8(scl_port, scl_pin, sda_port, sda_pin, device_addr << 1 | 1);
	nack += softi2c_read_nack(scl_port, scl_pin, sda_port, sda_pin);
	int data = softi2c_read8(scl_port, scl_pin, sda_port, sda_pin);
	softi2c_send_nack(scl_port, scl_pin, sda_port, sda_pin);
	softi2c_sig_stop(scl_port, scl_pin, sda_port, sda_pin);
	return data;
}

/**
 * Probes a device on the I2C bus using software I2C.
 *
 * @param scl_port The GPIO port for the SCL pin.
 * @param scl_pin The GPIO pin for the SCL pin.
 * @param sda_port The GPIO port for the SDA pin.
 * @param sda_pin The GPIO pin for the SDA pin.
 * @param device_addr The address of the device to probe.
 *
 * @returns True if the device is present on the bus, false otherwise.
 */
bool softi2c_probe(GPIO_TypeDef * scl_port, int scl_pin, GPIO_TypeDef * sda_port, int sda_pin, int device_addr) {
	int nack = 0;
	softi2c_sig_start(scl_port, scl_pin, sda_port, sda_pin);
	softi2c_send8(scl_port, scl_pin, sda_port, sda_pin, device_addr << 1 | 1);
	nack += softi2c_read_nack(scl_port, scl_pin, sda_port, sda_pin);
	softi2c_sig_stop(scl_port, scl_pin, sda_port, sda_pin);
	if (nack) {
		return 0;
	} else {
		return 1;
	}
}

/**
 * Reads a 16-bit register value from a device using software I2C protocol.
 *
 * @param scl_port The GPIO port of the SCL pin.
 * @param scl_pin The pin number of the SCL pin.
 * @param sda_port The GPIO port of the SDA pin.
 * @param sda_pin The pin number of the SDA pin.
 * @param device_addr The address of the device to read from.
 * @param high_reg_addr The address of the high byte of the register to read.
 * @param low_reg_addr The address of the low byte of the register to read.
 *
 * @returns The 16-bit register value read from the device.
 */
int16_t softi2c_read_reg_hl(GPIO_TypeDef * scl_port, int scl_pin, GPIO_TypeDef * sda_port, int sda_pin, int device_addr, int high_reg_addr, int low_reg_addr) {
	return softi2c_read_reg(scl_port, scl_pin, sda_port, sda_pin, device_addr, high_reg_addr) << 8 | softi2c_read_reg(scl_port, scl_pin, sda_port, sda_pin, device_addr, low_reg_addr);
}
