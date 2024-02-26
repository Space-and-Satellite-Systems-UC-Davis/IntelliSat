#include "ASM330LHH.h"
#include "../../globals.h"
#include "../../tools/print_scan.h"
#include "../../system_config/Buttons/buttons.h"


void gpio_set_T(GPIO_TypeDef * port, int pin, int value) {
	if (value) {
		port->BSRR = 1 << pin;
	} else {
		port->BSRR = 1 << (pin+16);
	}
}

void gpio_mode(GPIO_TypeDef * port, int pin, int mode, int open_drain, int speed, int pull) {
	port->MODER = (port->MODER & ~(0x3 << (pin*2))) | ((mode & 0x3) << (pin*2)); // 0 digital in with schmitt trigger, 1 output, 2 AF, 3 analog mode
	port->OTYPER = (port->OTYPER & ~(0x1 << pin)) | ((open_drain & 0x1) << pin); // 0 is normal push pull, 1 is open drain
	port->OSPEEDR = (port->OSPEEDR & ~(0x3 << (pin*2))) | ((speed & 0x3) << (pin*2)); // 0x3 is very high speed, 0x0 is normal low speed
	port->PUPDR = (port->PUPDR & ~(0x3 << (pin*2))) | ((pull & 0x3) << (pin*2)); // 0 no pupd, 1 pu, 2 pd, 3 reserved
}

void gpio_af(GPIO_TypeDef * port, uint8_t pin, uint8_t afn) {
	port->MODER = (port->MODER & ~(0x3 << (pin*2))) | ((0x2) << (pin*2));
	if (pin > 7) {
		port->AFR[1] = (port->AFR[1] & ~(0xF << ((pin-8)*4))) | ((afn & 0xF) << ((pin-8)*4));
	} else {
		port->AFR[0] = (port->AFR[0] & ~(0xF << (pin*4))) | ((afn & 0xF) << (pin*4));
	}
	gpio_low_T(port, pin);
}

void init_spi_master(SPI_TypeDef * spi,
	GPIO_TypeDef * mosi_port, int mosi_pin, int mosi_afn,
	GPIO_TypeDef * miso_port, int miso_pin, int miso_afn,
	GPIO_TypeDef * sck_port, int sck_pin, int sck_afn,
	GPIO_TypeDef * ncs_port, int ncs_pin) {

	// enable spi clocks
	switch ((uint32_t)spi) {
		case (uint32_t)SPI1:
			RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
			break;
		case (uint32_t)SPI2:
			RCC->APB1ENR1 |= RCC_APB1ENR1_SPI2EN;
			break;
		case (uint32_t)SPI3:
			RCC->APB1ENR1 |= RCC_APB1ENR1_SPI3EN;
			break;
	}

	gpio_af(mosi_port, mosi_pin, mosi_afn);
	gpio_af(miso_port, miso_pin, miso_afn);
	gpio_af(sck_port, sck_pin, sck_afn);
	gpio_set_T(ncs_port, ncs_pin, 1);
	gpio_mode(ncs_port, ncs_pin, 1, 0, 0, 0);

	spi->CR2 = 0x0700 // default value on reset.. ok this whole thing is redundant, default values are fine honestly
		| 1 << 12 // FRXTH: RXNE event generated if FIFO lvl >= 1/4 (for 8 bit)
		| 0x7 << 8 // data size: 8 bits
		| 0 << 7 // TXE interrupt enable = disable
		| 0 << 6 // RXNE i. disabled
		| 0 << 5 // ERR i. disabled
		| 0 << 4 // spi motorola mode
		| 0 << 3 // no nss pulse between consecutive frames
		| 0 << 2 // CS output disabled
		| 0 << 1 // tx dma disabled
		| 0 << 0; // rx dma disabled
	spi->CR1 = 0
		| 1 << 9 // software slave management enabled
		| 1 << 8 // internal slave selected
		| 0x7 << 2 // baud rate control: divide fpclk by 2^x
		| 1 << 2 // master mode
		| 0 << 1 // CPOL = 0: SCK to 0 when idle
		| 0 << 0 // CPHA = 0: data capture edge is first clock transition
		| 1 << 6; // enable SPI
}

void spi_block_while_busy(SPI_TypeDef * spi) {
	while (spi->SR & (1<<7)) {__NOP();} // wait while busy
}

void spi_tx8(SPI_TypeDef * spi, int data) {
	while (!(spi->SR & (1<<1))) {__NOP();} // wait while tx buffer is not empty
	*(volatile uint8_t*)&spi->DR = data & 0xFF;
	while (!(spi->SR & (1<<1))) {__NOP();} // wait while tx buffer is not empty before checking bsy
//	while (spi->SR & (1<<7)) {__NOP();} // wait while busy
//	return spi->DR; // if want to clear OVR flag, read DR then read SR. both fifo should be balanced here so im not doing that
}

uint16_t spi_rx8(SPI_TypeDef * spi) {
	return spi->DR;
}

void spi_tx8x2(SPI_TypeDef * spi, int data1, int data2) {
	while (!(spi->SR & (1<<1))) {__NOP();} // wait while tx buffer is not empty
	*(volatile uint8_t*)&spi->DR = data1 & 0xFF;
	while (!(spi->SR & (1<<1))) {__NOP();} // wait while tx buffer is not empty before checking bsy
//	while (spi->SR & (1<<7)) {__NOP();} // wait while busy
//	int hi = spi->DR; // dummy read
	*(volatile uint8_t*)&spi->DR = data2 & 0xFF;
	while (!(spi->SR & (1<<1))) {__NOP();} // wait while tx buffer is not empty before checking bsy
//	while (spi->SR & (1<<7)) {__NOP();} // wait while busy
//	hi = spi->DR;
//	return spi->DR;
}

void spi_tx8_long(SPI_TypeDef * spi, int len, int * txdata) {
	for (int i = 0; i < len; i++) {
		while (!(spi->SR & (1<<1))) {__NOP();} // wait while tx buffer is not empty
		*(volatile uint8_t*)&spi->DR = txdata[i] & 0xFF;
	}
	while (!(spi->SR & (1<<1))) {__NOP();} // wait while tx buffer is not empty before checking bsy
//	while (spi->SR & (1<<7)) {__NOP();} // wait while busy
	int temp = spi->DR; // clearing OVR flag with dummy DR and SR read
	temp = spi->SR;
}

void spi_txrx8_long(SPI_TypeDef * spi, int len, int * txdata, int * rxdata) {
	for (int i = 0; i < len; i++) {
		while (spi->SR & (1<<7)) {__NOP();} // wait while busy
		*(volatile uint8_t*)&spi->DR = txdata[i] & 0xFF;
		while (!(spi->SR & (1<<0))) {__NOP();} // wait while rx buf is empty
		rxdata[i] = spi->DR;
	}
}

#define OP2_IMU_SPI SPI3
#define OP2_IMU_SPI_NCS GPIOG, 15
#define OP2_IMU_SPI_PINS GPIOB, 5, 6, GPIOB, 4, 6, GPIOB, 3, 6, OP2_IMU_SPI_NCS

#define IMU_ODR_OFF 		0
#define IMU_ODR_12_5_Hz 	1
#define IMU_ODR_26_Hz 		2
#define IMU_ODR_52_Hz 		3
#define IMU_ODR_104_Hz 		4
#define IMU_ODR_208_Hz 		5
#define IMU_ODR_417_Hz 		6
#define IMU_ODR_833_Hz 		7
#define IMU_ODR_1667_Hz 	8
#define IMU_ODR_3333_Hz 	9
#define IMU_ODR_6667_Hz 	10
#define IMU_FS_2_g 			0
#define IMU_FS_4_g 			2
#define IMU_FS_8_g 			3
#define IMU_FS_16_g 		1
#define IMU_FS_125_dps 		2
#define IMU_FS_250_dps 		0
#define IMU_FS_500_dps 		4
#define IMU_FS_1000_dps 	8
#define IMU_FS_2000_dps 	12
#define IMU_FS_4000_dps 	1

void op2_imu_write8(int addr, int data) {
	gpio_set_T(OP2_IMU_SPI_NCS, 0);
	nop(10);
	spi_tx8x2(OP2_IMU_SPI, addr | (0<<7), data); // 0 is write 1 is read
	spi_rx8(OP2_IMU_SPI);
	spi_rx8(OP2_IMU_SPI);
	spi_rx8(OP2_IMU_SPI);
	spi_block_while_busy(OP2_IMU_SPI);
	nop(10);
	gpio_set_T(OP2_IMU_SPI_NCS, 1);
	nop(10);
}

int op2_imu_read8(int addr) {
	gpio_set_T(OP2_IMU_SPI_NCS, 0);
	nop(10);
	spi_tx8(OP2_IMU_SPI, addr | (1<<7)); // 0 is write 1 is read
	spi_rx8(OP2_IMU_SPI);
	spi_tx8(OP2_IMU_SPI, 0xFF); // tx dummy
	int data = spi_rx8(OP2_IMU_SPI);
	spi_block_while_busy(OP2_IMU_SPI);
	nop(10);
	gpio_set_T(OP2_IMU_SPI_NCS, 1);
	nop(10);
	return data;
}

int16_t op2_imu_read8x2(int addrl) {
	gpio_set_T(OP2_IMU_SPI_NCS, 0);
	nop(10);
	spi_tx8(OP2_IMU_SPI, addrl | (1<<7)); // 0 is write 1 is read
	spi_rx8(OP2_IMU_SPI);
	spi_tx8(OP2_IMU_SPI, 0xFF); // tx dummy
	int datal = spi_rx8(OP2_IMU_SPI);
	spi_tx8(OP2_IMU_SPI, 0xFF); // tx dummy
	int datah = spi_rx8(OP2_IMU_SPI);
	spi_block_while_busy(OP2_IMU_SPI);
	nop(10);
	gpio_set_T(OP2_IMU_SPI_NCS, 1);
	nop(10);
	return (datah << 8) | datal;
}

void op2_imu_acel_ctrl(int acel_rate, int acel_scale, int digital_filter_on) {
	acel_rate &= 0xFF;
	acel_scale &= 0xF;
	digital_filter_on &= 1;
	int data = acel_rate << 4 | acel_scale << 2 | digital_filter_on << 1;
	op2_imu_write8(0x10, data);
}

void op2_imu_gyro_ctrl(int gyro_rate, int gyro_scale) {
	gyro_rate &= 0xFF;
	gyro_scale &= 0xFF;
	int data = gyro_rate << 4 | gyro_scale;
	op2_imu_write8(0x11, data);
}

//void imu_init(int acel_rate, int acel_scale, int gyro_rate, int gyro_scale) {
//	init_spi_master(OP2_IMU_SPI, OP2_IMU_SPI_PINS);
//	op2_imu_write8(0x12, 0x1); // soft reset imu
//	nop(500);
//	op2_imu_acel_ctrl(acel_rate, acel_scale, 0);
//	op2_imu_gyro_ctrl(gyro_rate, gyro_scale);
//}

//int16_t imu_readAcel_X() {
//	return op2_imu_read8x2(0x28);
//}

//int16_t imu_readAcel_Y() {
//	return op2_imu_read8x2(0x2A);
//}
//
//int16_t imu_readAcel_Z() {
//	return op2_imu_read8x2(0x2C);
//}
//
//int16_t imu_readGyro_X() {
//	return op2_imu_read8x2(0x22);
//}
//
//int16_t imu_readGyro_Y() {
//	return op2_imu_read8x2(0x24);
//}
//
//int16_t imu_readGyro_Z() {
//	return op2_imu_read8x2(0x26);
//}


void branch_main() {
	imu_init(IMU_ODR_3333_Hz, IMU_FS_2_g, IMU_ODR_3333_Hz, IMU_FS_1000_dps);

	float acel_X = 0;
	float acel_Y = 0;
	float acel_Z = 0;
	float gyro_X = 0;
	float gyro_Y = 0;
	float gyro_Z = 0;

	while (1) {
		if (get_buttonStatus_1()) {
			acel_X = ((int16_t)imu_readAcel_X() * 0.061) / 1000;
			acel_Y = ((int16_t)imu_readAcel_Y() * 0.061) / 1000;
			acel_Z = ((int16_t)imu_readAcel_Z() * 0.061) / 1000;

			gyro_X = ((int16_t)imu_readGyro_X() * 35.0) / 1000;
			gyro_Y = ((int16_t)imu_readGyro_Y() * 35.0) / 1000;
			gyro_Z = ((int16_t)imu_readGyro_Z() * 35.0) / 1000;
		}
	}
}





















