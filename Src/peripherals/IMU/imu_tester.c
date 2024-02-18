#include "ASM330LHH.h"
#include "../../globals.h"
#include "../../tools/print_scan.h"
#include "../../system_config/Buttons/buttons.h"

//void branch_main() {
//	imu_init(IMU_ODR_3333_Hz, IMU_FS_2_g, IMU_ODR_3333_Hz, IMU_FS_1000_dps);
//
//	int16_t read_val = 0;
//	while (1) {
//		read_val = imu_readAcel_X();
//		printMsg("%d\r\n", read_val);
//
//		nop(10000000);	// random delay to slow down stuff a bit
//	}
//}

int core_h = 0; // please update when configuring pll. not really used rn
void init_clocks() {
	// read chip identification information. only useful when watching memory through debugger
	volatile int id_w_x = (*(uint32_t*)(UID_BASE) & 0xFFFF0000) >> 16;
	volatile int id_w_y = *(uint32_t*)(UID_BASE) & 0xFFFF;
	volatile int id_w_n = *(uint32_t*)(UID_BASE+0x4) & 0xFF;
	volatile int id_lot_l = *(uint32_t*)(UID_BASE+0x4) & 0xFFFFFF00;
	volatile int id_lot_h = *(uint32_t*)(UID_BASE+0x8);
	volatile char id_lot[7] = {
		(id_lot_l >> 8) & 0xFF,
		(id_lot_l >> 16) & 0xFF,
		(id_lot_l >> 24) & 0xFF,
		(id_lot_h) & 0xFF,
		(id_lot_h >> 8) & 0xFF,
		(id_lot_h >> 16) & 0xFF,
		(id_lot_h >> 24) & 0xFF
	};
	RCC->AHB2ENR = 0x000530FF; // enable clock to all gpio banks, usb otg, adc, crypto
	RCC->AHB3ENR = 0x00000100; // enable clock to QSPI
	RCC->APB1ENR1 =
		1 << 28 // enable power control
		| 1 << 22 // enable i2c2
		| 1 << 15 // enable spi3
		| 1 << 14 // enable spi2
		| 1 << 10; // enable rtc apb
	RCC->APB1ENR2 =
		1 << 0; // enable lpuart1
	RCC->APB2ENR = 1; // enable syscfg
	FLASH->ACR = 0x600 | 4 | 1<<8; // flash access latency to four wait states, prefetch enable lol
	//FLASH->ACR = 0x604; // OVERCLOCK! for testing only
	RCC->CR |= 1 << 8; // enable HSI
	RCC->PLLCFGR =
		0x0 << 25 // PLLR = 2
		| 1 << 24 // PLLREN = 1
		| 0x0 << 21 // PLLQ = 2
		| 1 << 20 // PLLQEN (PLL48M1CLK) output enabled
		| 0x0 << 17 // PLLP default
		| 0 << 16 // PLLSAI3 not enabled
		 | 10 << 8 // PLLN = 10
	//	| 20 << 8 // OVERCLOCK! for testing only
		| 0 << 4 // PLLM = 1
		| 0x2; // HSI16 as input clock to PLLs
	RCC->PLLSAI1CFGR =
		0x1 << 25 // PLLSAI1R = 4
		| 1 << 24 // PLLSAI1REN = PLLADC1CLK enable
		| 0x1 << 21 // PLLSAI1Q = 4
		| 1 << 20 // PLLSAI1QEN = PLL48M2CLK enable
		| 0x0 << 17 // PLLSAI1P = default
		| 0 << 16 // PLLSAI1PEN = 0
		| 12 << 8; // PLLSAI1N = 12
	RCC->CR |= (1 << 24) | (1 << 26); // enable PLL and PLLSAI1
	while (!(RCC->CR & (1 << 25))); // wait for PLL lock
	while (!(RCC->CR & (1 << 27))); // wait for PLLSAI1 lock
	RCC->CFGR = 0x3; // system clock to PLL
	core_h = 80;
}

void gpio_high_T(GPIO_TypeDef * port, int pin) {
	port->BSRR = 1 << pin;
}

void gpio_low_T(GPIO_TypeDef * port, int pin) {
	port->BSRR = 1 << (pin+16);
}

void gpio_set_T(GPIO_TypeDef * port, int pin, int value) {
	if (value) {
		gpio_high_T(port, pin);
	} else {
		gpio_low_T(port, pin);
	}
}

int gpio_read_T(GPIO_TypeDef * port, int pin) {
	if (port->IDR & (1 << pin)) {
		return 1;
	} else {
		return 0;
	}
}


#define GPIO_DIGITAL_IN		0
#define GPIO_DIGITAL_OUT	1
#define GPIO_AF 			2
#define GPIO_ANALOG_IN 		3
#define GPIO_PUSH_PULL 		0
#define GPIO_OPEN_DRAIN		1
#define GPIO_LOW_SPEED		0
#define GPIO_MED_SPEED		1
#define GPIO_HI_SPEED		2
#define GPIO_VERY_HI_SPEED	3
#define GPIO_NO_PULL		0
#define GPIO_PULL_UP		1
#define GPIO_PULL_DOWN		2

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

void init_gpio() {
	PWR->CR2 |= 1 << 9; // enable VDDIO2 supply for GPIOG

	// wait until each GPIO is clocked and ready:
	// (there is a delay before perph is enabled after RCC to synchronize internal bus stuff)
	while (GPIOA->OTYPER == 0xFFFFFFFF);
	while (GPIOB->OTYPER == 0xFFFFFFFF);
	while (GPIOC->OTYPER == 0xFFFFFFFF);
	while (GPIOD->OTYPER == 0xFFFFFFFF);
	while (GPIOE->OTYPER == 0xFFFFFFFF);
	while (GPIOF->OTYPER == 0xFFFFFFFF);
	while (GPIOG->OTYPER == 0xFFFFFFFF);

	gpio_mode(GPIOB, 10, 0, 0, 0, 1); // BTNA - input with schmitt trigger, not open drain, low speed, pull up
	gpio_mode(GPIOB, 11, 0, 0, 0, 1); // BTNB

	gpio_mode(GPIOE, 6, 1, 0, 0, 0); // LED0 - output, not OD, low speed, no pull
	gpio_mode(GPIOE, 5, 1, 0, 0, 0); // LED1
	gpio_mode(GPIOE, 4, 1, 0, 0, 0); // LED2
	gpio_mode(GPIOE, 3, 1, 0, 0, 0); // LED3
	gpio_mode(GPIOE, 2, 1, 0, 0, 0); // LED4
}

void init_spi_master(SPI_TypeDef * spi,
	GPIO_TypeDef * mosi_port, int mosi_pin, int mosi_afn,
	GPIO_TypeDef * miso_port, int miso_pin, int miso_afn,
	GPIO_TypeDef * sck_port, int sck_pin, int sck_afn,
	GPIO_TypeDef * ncs_port, int ncs_pin) {
	// assume peripheral is already enabled in RCC

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

void imu_init(int acel_rate, int acel_scale, int gyro_rate, int gyro_scale) {
	init_spi_master(OP2_IMU_SPI, OP2_IMU_SPI_PINS);
	op2_imu_write8(0x12, 0x1); // soft reset imu
	nop(500);
	op2_imu_acel_ctrl(acel_rate, acel_scale, 0);
	op2_imu_gyro_ctrl(gyro_rate, gyro_scale);
}

//int16_t imu_readAcel_X() {
//	return op2_imu_read8x2(0x28);
//}

int16_t imu_readAcel_Y() {
	return op2_imu_read8x2(0x2A);
}

int16_t imu_readAcel_Z() {
	return op2_imu_read8x2(0x2C);
}

int16_t imu_readGyro_X() {
	return op2_imu_read8x2(0x22);
}

int16_t imu_readGyro_Y() {
	return op2_imu_read8x2(0x24);
}

int16_t imu_readGyro_Z() {
	return op2_imu_read8x2(0x26);
}


void op2_core_init() { // not really necessary to group like this but it fits better with all the op-specific inits
	init_clocks();
	init_gpio();
}

void branch_main() {
	op2_core_init();

	imu_init(IMU_ODR_3333_Hz, IMU_FS_2_g, IMU_ODR_3333_Hz, IMU_FS_1000_dps);

	float acel_X = 0;
	float acel_Y = 0;
	float acel_Z = 0;
	float gyro_X = 0;
	float gyro_Y = 0;
	float gyro_Z = 0;

	while (1) {
		if (get_buttonStatus_1()) {
			acel_X = ((int16_t)imu_readAcel_X() * 0.061) / 1000;	// THIS IS USING THE IntelliSat SPI
			acel_Y = ((int16_t)imu_readAcel_Y() * 0.061) / 1000;
			acel_Z = ((int16_t)imu_readAcel_Z() * 0.061) / 1000;

			gyro_X = ((int16_t)imu_readGyro_X() * 35.0) / 1000;
			gyro_Y = ((int16_t)imu_readGyro_Y() * 35.0) / 1000;
			gyro_Z = ((int16_t)imu_readGyro_Z() * 35.0) / 1000;
		}
	}
}





















