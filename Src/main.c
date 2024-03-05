// opr2-quickstart1
// Quick start for Orbital Platform R2
// TCS 202310. Yume Research & Friends.
// The above attribution must be included regardless of license.
// This is not permissively licensed software.

//

#include <stm32l476xx.h>
#include <math.h> // for sqrt
#include "hdd/pid.h"
typedef enum { false, true } bool;

#include <string.h>
/* reverse:  reverse string s in place */
static void reverse(char s[]) {
	int i, j;
	char c;

	for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}

 /* itoa:  convert n to characters in s */
static void itoa(int n, char s[]) {
	int i, sign;
	if ((sign = n) < 0)  /* record sign */
		n = -n;          /* make n positive */
	i = 0;
	do {       /* generate digits in reverse order */
		s[i++] = n % 10 + '0';   /* get next digit */
	} while ((n /= 10) > 0);     /* delete it */
	if (sign < 0)
		s[i++] = '-';
	s[i] = '\0';
	reverse(s);
}

unsigned int abs16(int16_t a) {
	if (a < 0) {
		return (a * -1);
	} else {
		return a;
	}
}

void nop(int nop_loops) {
	for (int i = 0; i < nop_loops; i++) {__NOP();}
}

void nop_ms(int ms) {
	nop(80 * 250 * ms); // assume 80 MHz
}

int core_MHz = 0; // please update when configuring pll. not really used rn
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
	core_MHz = 80;
}

void gpio_high(GPIO_TypeDef * port, int pin) {
	port->BSRR = 1 << pin;
}

void gpio_low(GPIO_TypeDef * port, int pin) {
	port->BSRR = 1 << (pin+16);
}

void gpio_set(GPIO_TypeDef * port, int pin, int value) {
	if (value) {
		gpio_high(port, pin);
	} else {
		gpio_low(port, pin);
	}
}

int gpio_read(GPIO_TypeDef * port, int pin) {
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
	gpio_low(port, pin);
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

unsigned int systick_time = 0;
void SysTick_Handler() {
	systick_time++;
}

void delay_ms(unsigned int ms) {
	if (!ms) return;
	unsigned int target = systick_time + ms;
	while (target < systick_time) {__NOP();}; // in case overflow and wrapping
	while (target > systick_time) {__NOP();};
}

void init_nvic() {
	__disable_irq();
	SysTick->LOAD = core_MHz/8*1000; // configure for 1 ms period, use AHB/8
	SysTick->CTRL = 0x3; // use AHB/8 as input clock, enable interrupts and counter
	NVIC_EnableIRQ(SysTick_IRQn);
	__enable_irq();
}

// i just implemented against lpuart. brr calculation for normal u(s)art might differ
void init_lpuart(USART_TypeDef * uart,
	GPIO_TypeDef * tx_port, int tx_pin, int tx_afn,
	GPIO_TypeDef * rx_port, int rx_pin, int rx_afn) {
	// assume peripheral is already enabled in RCC

	gpio_af(tx_port, tx_pin, tx_afn);
	gpio_af(rx_port, rx_pin, rx_afn);

	uart->BRR = 177778; // approx 115200 with 80 MHz: (256·80MHz)/x = 115200Hz
	uart->CR3 = 0; // not really using anything here
	uart->CR2 = 0 << 12; // 1 stop bit.. the defaults are fine here again
	uart->CR1 =
		(0 << 28)|(0 << 12) // 1 start bits, 8 data bits, n stop bits
		| 1 << 3 // transmitter enable. this generates a preamble immediately
		| 1 << 2 // receiver enable lol
		| 1 << 0; // enable uart
}

void uart_tx_char(USART_TypeDef * uart, uint8_t data) {
	while (!(uart->ISR & 1<<7)) {__NOP();}; // while not TXE
	uart->TDR = data;
}

void uart_tx_long(USART_TypeDef * uart, int len, uint8_t * data) {
	for (int i = 0; i < len; i++) {
		while (!(uart->ISR & 1<<7)) {__NOP();}; // while not TXE
		uart->TDR = data[i];
	}
	while (!(uart->ISR & 1<<6)) {__NOP();}; // while not TC
}

//UNUSED! (wanted to make sure we can hit 115200 baud for spin demo)
void init_softuart(GPIO_TypeDef * tx_port, int tx_pin, GPIO_TypeDef * rx_port, int rx_pin) {
	gpio_set(tx_port, tx_pin, 1);
	gpio_mode(tx_port, tx_pin, GPIO_DIGITAL_OUT, GPIO_PUSH_PULL, GPIO_HI_SPEED, GPIO_NO_PULL);
	gpio_mode(rx_port, rx_pin, GPIO_DIGITAL_IN, 0, GPIO_HI_SPEED, GPIO_PULL_DOWN);
}

//UNUSED:
void softuart_tx_char(GPIO_TypeDef * tx_port, int tx_pin, GPIO_TypeDef * rx_port, int rx_pin, int data) {

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
	gpio_set(ncs_port, ncs_pin, 1);
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

int spi_block_while_busy(SPI_TypeDef * spi) {
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

void softi2c_line_mode(GPIO_TypeDef * port, int pin, bool deassert_line) {
	if (deassert_line) {
		port->MODER &= ~(0x3 << (pin*2));
	} else {
		port->MODER = (port->MODER & ~(0x3 << (pin*2))) | (1 << pin*2);
	}
}

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

void softi2c_delay() {
	nop(30);
}

void softi2c_sig_start(GPIO_TypeDef * scl_port, int scl_pin, GPIO_TypeDef * sda_port, int sda_pin) {
	softi2c_line_mode(scl_port, scl_pin, 1);
	softi2c_line_mode(sda_port, sda_pin, 1);
	softi2c_delay(); softi2c_delay();
	softi2c_line_mode(sda_port, sda_pin, 0);
	softi2c_delay(); softi2c_delay();
	softi2c_line_mode(scl_port, scl_pin, 0);
	softi2c_delay();
}

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

void softi2c_sig_stop(GPIO_TypeDef * scl_port, int scl_pin, GPIO_TypeDef * sda_port, int sda_pin) {
	softi2c_line_mode(scl_port, scl_pin, 0);
	softi2c_line_mode(sda_port, sda_pin, 0);
	softi2c_delay(); softi2c_delay();
	softi2c_line_mode(scl_port, scl_pin, 1);
	softi2c_delay(); softi2c_delay();
	softi2c_line_mode(sda_port, sda_pin, 1);
	softi2c_delay();
}

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

void softi2c_send_nack(GPIO_TypeDef * scl_port, int scl_pin, GPIO_TypeDef * sda_port, int sda_pin) {
	softi2c_line_mode(scl_port, scl_pin, 0);
	softi2c_delay();
	softi2c_line_mode(sda_port, sda_pin, 1);
	softi2c_delay();
	softi2c_line_mode(scl_port, scl_pin, 1);
	softi2c_delay();
}

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

int16_t softi2c_read_reg_hl(GPIO_TypeDef * scl_port, int scl_pin, GPIO_TypeDef * sda_port, int sda_pin, int device_addr, int high_reg_addr, int low_reg_addr) {
	return softi2c_read_reg(scl_port, scl_pin, sda_port, sda_pin, device_addr, high_reg_addr) << 8 | softi2c_read_reg(scl_port, scl_pin, sda_port, sda_pin, device_addr, low_reg_addr);
}

void init_adc() {
	GPIOA->MODER |= 0x3 << (1*2);

}

#define OP2_SCI_UART LPUART1
#define OP2_SCI_UART_PINS GPIOC, 1, 8, GPIOC, 0, 8
//#define OP2_SCI_UART GPIOC, 1, GPIOC, 0
#define OP2_IMU_SPI SPI3
#define OP2_IMU_SPI_NCS GPIOG, 15
#define OP2_IMU_SPI_PINS GPIOB, 5, 6, GPIOB, 4, 6, GPIOB, 3, 6, OP2_IMU_SPI_NCS
#define OP2_MAG_I2C GPIOB, 8, GPIOB, 9
#define MAG_ADDR 0x0D

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

#define MAG_ODR_OFF 		0
#define MAG_ODR_10_Hz 		1
#define MAG_ODR_50_Hz 		2
#define MAG_ODR_100_Hz 		3
#define MAG_ODR_200_Hz		4
#define MAG_FS_2_G 			0 // btw: "G" is not a typo, it stands for gauss
#define MAG_FS_8_G	 		1
#define MAG_OVERSAMPLE_512 	0
#define MAG_OVERSAMPLE_256 	1
#define MAG_OVERSAMPLE_128 	2
#define MAG_OVERSAMPLE_64  	3

void op2_imu_write8(int addr, int data) {
	gpio_set(OP2_IMU_SPI_NCS, 0);
	nop(10);
	spi_tx8x2(OP2_IMU_SPI, addr | (0<<7), data); // 0 is write 1 is read
	spi_rx8(OP2_IMU_SPI);
	spi_rx8(OP2_IMU_SPI);
	spi_rx8(OP2_IMU_SPI);
	spi_block_while_busy(OP2_IMU_SPI);
	nop(10);
	gpio_set(OP2_IMU_SPI_NCS, 1);
	nop(10);
}

int op2_imu_read8(int addr) {
	gpio_set(OP2_IMU_SPI_NCS, 0);
	nop(10);
	spi_tx8(OP2_IMU_SPI, addr | (1<<7)); // 0 is write 1 is read
	spi_rx8(OP2_IMU_SPI);
	spi_tx8(OP2_IMU_SPI, 0xFF); // tx dummy
	int data = spi_rx8(OP2_IMU_SPI);
	spi_block_while_busy(OP2_IMU_SPI);
	nop(10);
	gpio_set(OP2_IMU_SPI_NCS, 1);
	nop(10);
	return data;
}

int16_t op2_imu_read8x2(int addrl) {
	gpio_set(OP2_IMU_SPI_NCS, 0);
	nop(10);
	spi_tx8(OP2_IMU_SPI, addrl | (1<<7)); // 0 is write 1 is read
	spi_rx8(OP2_IMU_SPI);
	spi_tx8(OP2_IMU_SPI, 0xFF); // tx dummy
	int datal = spi_rx8(OP2_IMU_SPI);
	spi_tx8(OP2_IMU_SPI, 0xFF); // tx dummy
	int datah = spi_rx8(OP2_IMU_SPI);
	spi_block_while_busy(OP2_IMU_SPI);
	nop(10);
	gpio_set(OP2_IMU_SPI_NCS, 1);
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

void op2_imu_init(int acel_rate, int acel_scale, int gyro_rate, int gyro_scale) {
	init_spi_master(OP2_IMU_SPI, OP2_IMU_SPI_PINS);
	op2_imu_write8(0x12, 0x1); // soft reset imu
	nop(500);
	op2_imu_acel_ctrl(acel_rate, acel_scale, 0);
	op2_imu_gyro_ctrl(gyro_rate, gyro_scale);
}

int16_t op2_imu_read_whoami() {
	return op2_imu_read8(0x0F);
}

int16_t op2_imu_read_acel_x() {
	return op2_imu_read8x2(0x28);
}

int16_t op2_imu_read_acel_y() {
	return op2_imu_read8x2(0x2A);
}

int16_t op2_imu_read_acel_z() {
	return op2_imu_read8x2(0x2C);
}

int16_t op2_imu_read_gyro_x() {
	return op2_imu_read8x2(0x22);
}

int16_t op2_imu_read_gyro_y() {
	return op2_imu_read8x2(0x24);
}

int16_t op2_imu_read_gyro_z() {
	return op2_imu_read8x2(0x26);
}

int16_t op2_imu_read_temp() {
	return op2_imu_read8x2(0x20);
}

void op2_mag_ctrl(int rate, int scale, int oversample) {
	if (rate) {
		softi2c_write_reg(OP2_MAG_I2C, MAG_ADDR, 0x09, oversample << 6 | scale << 4 | (rate-1) << 2 | 1);
	} else {
		softi2c_write_reg(OP2_MAG_I2C, MAG_ADDR, 0x09, 0);
	}
}

void op2_mag_init(int rate, int scale, int oversample) {
	init_softi2c(OP2_MAG_I2C);
	softi2c_write_reg(OP2_MAG_I2C, MAG_ADDR, 0x0A, 0x80); // soft reset
	nop(50);
	softi2c_write_reg(OP2_MAG_I2C, MAG_ADDR, 0x0A, 0x00);
	softi2c_write_reg(OP2_MAG_I2C, MAG_ADDR, 0x0B, 1);
	nop(50);
	op2_mag_ctrl(rate, scale, oversample);
}

int16_t op2_mag_read_x() {
	return softi2c_read_reg_hl(OP2_MAG_I2C, MAG_ADDR, 0x01, 0x00);
}

int16_t op2_mag_read_y() {
	return softi2c_read_reg_hl(OP2_MAG_I2C, MAG_ADDR, 0x03, 0x02);
}

int16_t op2_mag_read_z() {
	return softi2c_read_reg_hl(OP2_MAG_I2C, MAG_ADDR, 0x05, 0x04);
}

int16_t op2_mag_read_temp() {
	return softi2c_read_reg_hl(OP2_MAG_I2C, MAG_ADDR, 0x08, 0x07);
}

void op2_sci_uart_init() {
	init_lpuart(OP2_SCI_UART, OP2_SCI_UART_PINS);
}

void op2_printstr(char data[]) {
	for (int i = 0; i < strlen(data); i++) {
		uart_tx_char(OP2_SCI_UART, data[i]);
	}
}

char textbuf[255] = {0};

void op2_printint16(int16_t value) {
	if (value < 0) uart_tx_char(OP2_SCI_UART, '-');
	itoa(abs16(value), textbuf);
	op2_printstr(textbuf);
}

void op2_printuint32(uint32_t value) {
	itoa(value, textbuf);
	op2_printstr(textbuf);
}

void op2_printnewline() {
	uart_tx_char(OP2_SCI_UART, '\r');
	uart_tx_char(OP2_SCI_UART, '\n');
}

void op2_core_init() { // not really necessary to group like this but it fits better with all the op-specific inits
	init_clocks();
	init_gpio();
	init_nvic();
}

int maxAbs(x, y){
	if (x*x <= y*y){
		return x;
	} else {
		return y;
	}
}

int zero(x){
	if (x > 0){
		return x;
	}
	return 0;
}
/*
#define ESC_PERIOD_MICROSEC      5000
#define ESC_MIN_DUTYCYCLE_PERCNT 10
#define ESC_MID_DUTYCYCLE_PERCNT 15
#define ESC_MAX_DUTYCYCLE_PERCNT 20

#define PID_KP 1.0f
#define PID_KI 0.0f
#define PID_KD 0.0f

#define IMU_TIME 0.003f
#define PID_TIME 0.0003f //10 times faster than IMU time

#define PID_TAU 0.8f //value between 0 and 1, low pass filter

#define PID_INTEGRATOR_MIN 0.0f
#define PID_INTEGRATOR_MAX 0.0f

#define PID_OUTPUT_MIN 0.0f
#define PID_OUTPUT_MAX 200.0f

#define PID_SETPOINT 150.0f

#define GYRO_RATE 3333
#define GYRO_SCALE 4000
*/
int main() {
	op2_core_init();

	gpio_set(GPIOE, 2, 1);

	op2_sci_uart_init();
	op2_imu_init(IMU_ODR_3333_Hz, IMU_FS_2_g, IMU_ODR_3333_Hz, IMU_FS_1000_dps);
	op2_mag_init(MAG_ODR_200_Hz, MAG_FS_8_G, MAG_OVERSAMPLE_512);

	int state = 0;
	int btn_debounce = 0;

	gpio_set(GPIOE, 2, 0);

	// output factory info
	op2_printstr("orbital platform r2. mcu serial bytes: ");
	op2_printuint32(*(uint32_t*)(UID_BASE));
	op2_printstr(", ");
	op2_printuint32(*(uint32_t*)(UID_BASE+0x4));
	op2_printstr(", ");
	op2_printuint32(*(uint32_t*)(UID_BASE+0x8));
	op2_printnewline();

	op2_imu_read_whoami();

	/*PIDController pid;
	PIDController* IMUPID = &pid;
	PIDController_Init(&pid);
	//Parameters: PID, Kp, Ki, Kd, Time(10 times faster than IMU measurement cycle, in SECONDS)
	PIDController_SetLimits(&pid, 1.2f, -0.5f, 0.5f, -5.0f, 5.0f);
	PIDController_SetTerms(&pid, 1.0f, 0.01f, 0.0f, 1.0f);*/
	//PIDController_SetTerms(IMUPID, PID_KP, PID_KI, PID_KD, PID_TIME);//Last parameter, time, should be 10 times less than the IMU period
	//Parameters: PID

	//Parameters: PID, low pass filter constant tau (between 0 and 1), integrator min, integrator max, output min, output max
	//PIDController_SetLimits(IMUPID, PID_TAU, PID_INTEGRATOR_MIN, PID_INTEGRATOR_MAX, PID_OUTPUT_MIN, PID_OUTPUT_MAX);

	int btna_pressed_at_start = gpio_read(GPIOB, 10);

	if (!btna_pressed_at_start) {gpio_set(GPIOE, 4, 1);}  // diagnostic mode if btna is pressed
	else {
		gpio_set(GPIOE, 2, 1);
		gpio_set(GPIOE, 3, 1);
	}

	while (1) {
	//	if (btna_pressed_at_start) {gpio_af(GPIOA, 15, 1);} // output pwm if NOT in diagnostic mode
		gpio_af(GPIOA, 15, 1);
		RCC->APB1ENR1 |= 1<<0; 					// enable TIM2
		delay_ms(1);
		TIM2->PSC = 81; 						// clock prescaler
		TIM2->CCER = 1<<0;						//
		TIM2->CCMR1 = (6 << 4) | (1 << 3);	// pwm mode, enable
		TIM2->EGR = 1<<1; 						// give an event trigger to update shadow registers
		TIM2->ARR = 10000;						// reload register
		TIM2->CCR1 = 1000;
		TIM2->CR1 =
			(1 << 7) | 1; 						// enable auto reload preload and enable tim2

		TIM2->CCR1 = 1500; // rc protocol: 1 ms is minimum, 2 ms is maximum

		int th = 0 * 5; // esc throttle, plus or minus 100
		TIM2->CCR1 = 1500 + th * 5; // this is for yellow/normal hdd stuff
	//	TIM2->CCR1 = 1000 + th * 5; // for spider esc

	//	while(1);

		int targetms = 10; // delay before starting program
		while (1) {
			if (systick_time >= targetms) {
				op2_printstr("cannot keep up! reduce output data rate.\r\n");
				op2_printstr("limited by uart baud rate.\r\n");
				while (1) __NOP();
			}
			th = 100;

			if (systick_time> 5000) {
				//th = 100 + (systick_time - 5000) / 100;
				th = maxAbs(zero(100-(op2_imu_read_gyro_z()+1)*5), 150);
				//th = (int)PIDController_Update(pid*, 150, -(float)op2_imu_read_gyro_z());
				// 5 seconds: take 10 seconds to ramp to full throttle
			}/*
			if (systick_time > 15000) {
				th = 200;
				// 15 seconds: full throttle
			}
			if (systick_time > 20000) {
				th = 100;
				// 20 seconds: zero throttle
			}
			if (systick_time> 30000) {
				th = 100 - (systick_time - 30000) / 100;
				// 5 seconds: take 10 seconds to ramp to full throttle
			}
			if (systick_time > 40000) {
				th = 0;
				// 20 seconds: zero throttle
			}
			if (systick_time > 45000) {
				th = 100;
				// 20 seconds: zero throttle
			}
			if (systick_time> 55000) {
				th = 100 + (systick_time - 55000) / 500;
				// 5 seconds: take 10 seconds to ramp to full throttle
			}
			if (systick_time > 65000) {
				th = 120;
				// 15 seconds: full throttle
			}
			if (systick_time > 70000) {
				th = 100;
				// 20 seconds: zero throttle
			}
			if (systick_time > 75000) {
				th = 100 - (systick_time - 75000) / 500;
				// 5 seconds: take 10 seconds to ramp to full throttle
			}
			if (systick_time > 85000) {
				th = 80;
				// 20 seconds: zero throttle
			}
			if (systick_time > 90000) {
				th = 100;
				// 20 seconds: zero throttle
			}*/
			if (!btna_pressed_at_start) th = 0;
			//TIM2->CCR1 = 1500 + th * 5; // this is for yellow/normal hdd stuff
			TIM2->CCR1 = 1000 + th * 5; // for spider esc

			while (systick_time < targetms) __NOP();

			op2_printuint32(systick_time);
			/*op2_printstr(",");
			op2_printint16(op2_imu_read_acel_x());
			op2_printstr(",");
			op2_printint16(op2_imu_read_acel_y());
			op2_printstr(",");
			op2_printint16(op2_imu_read_acel_z());
			op2_printstr(",");
			op2_printint16(op2_imu_read_gyro_x());
			op2_printstr(",");
			op2_printint16(op2_imu_read_gyro_y());*/
			op2_printstr(",");
			op2_printint16(op2_imu_read_gyro_z()+1);
			op2_printstr(",");
			op2_printuint32(th);
			op2_printnewline();


			targetms += 5;
		}
	}




// 	while (1) {
// 		op2_print_sci("good evening! the current systick is: ");
// 		itoa(systick_time, textbuf);
// 		op2_print_sci(textbuf);
// 		op2_print_sci(" ms\r\n");

// 		op2_imu_read_whoami();
// 		op2_imu_read_acel_x();
// 		gpio_set(GPIOE, 6, 1);
// //		delay_ms(500);
// 		gpio_set(GPIOE, 6, 0);
// //		delay_ms(500);
// 	}

	while (0) { // blinky demo
		gpio_set(GPIOE, 2, 1);
		gpio_set(GPIOE, 3, 0);
		delay_ms(60);
		gpio_set(GPIOE, 3, 1);
		gpio_set(GPIOE, 2, 0);
		delay_ms(60);
		gpio_set(GPIOE, 4, 1);
		gpio_set(GPIOE, 3, 0);
		delay_ms(60);
		gpio_set(GPIOE, 5, 1);
		gpio_set(GPIOE, 4, 0);
		delay_ms(60);
		gpio_set(GPIOE, 6, 1);
		gpio_set(GPIOE, 5, 0);
		delay_ms(60);
		gpio_set(GPIOE, 5, 1);
		gpio_set(GPIOE, 6, 0);
		delay_ms(60);
		gpio_set(GPIOE, 4, 1);
		gpio_set(GPIOE, 5, 0);
		delay_ms(60);
		gpio_set(GPIOE, 3, 1);
		gpio_set(GPIOE, 4, 0);
		delay_ms(60);
	}

	// below is legacy test programs for OPR1

	while (1) { 								// test program: tilt throttle
		// timing calculations:
		// at full 80 MHz HSI, APB1 runs at same 80 MHz
		// PWM freq = APB1 / PSC / ARR
		// 80 MHz / 80 [PSC] / 10000 [ARR] = 100 Hz

		GPIOD->MODER |= 2 << (15*2); 			// configure gpio afr to tim4 ch4
		GPIOD->AFR[1] = 2 << (7*4);

		RCC->APB1ENR1 |= 1<<2; 					// enable TIM4

		TIM4->PSC = 80; 						// clock prescaler
		TIM4->CCER = 1<<12;						//
		TIM4->CCMR2 = (6 << 12) | (1 << 11);	// pwm mode, enable
		TIM4->EGR = 1<<4; 						// give an event trigger to update shadow registers
		TIM4->ARR = 10000;						// reload register
		TIM4->CCR4 = 1000;
		TIM4->CR1 =
			(1 << 7) | 1; 						// enable auto reload preload and enable tim4


		while (1) { // pre programmed program for rig testing on 2023 10 28
			gpio_set(GPIOG, 9, 1);
			TIM4->CCR4 = 1500; // rc protocol: 1 ms is minimum, 2 ms is maximum
			nop(0x6FFFFFF);
			gpio_set(GPIOG, 9, 0);
			TIM4->CCR4 = 2000; // rc protocol: 1 ms is minimum, 2 ms is maximum
			nop(0x6FFFFFF);
			while (1) {
				for (int i = 0; i < 1000; i++) {
					TIM4->CCR4 = 1000 + i;
					nop(0xFFFF);
				}
			}
		}

		// while (1) { // tilt throttle loop
		// 	int th = abs16(op1_imu_read_acel_x());
		// 	GPIOD->ODR = (GPIOD->ODR & 0xFFFFFF00) | (th>>8) & 0xFF;
		// 	int th_scale = th * 10000 / 0xFFFF * 4; // convert acel reading to gyro. multiply by full scale of acel sensor (4)
		// 	TIM4->CCR4 = 1000 + th_scale / 10; // rc protocol: 1 ms is minimum, 2 ms is maximum
		// 	nop(10000);
		// }

	}










}





