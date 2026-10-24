#include "INA3221.h"
#include "I2C/i2c.h"
#include "GPIO/gpio.h"

void testFunction_INA3221() {
//	ina3221_init(0, 4, 4, 68, 30);

//	gpio_mode(
//			INA3221_SDA_PORT,
//			INA3221_SDA_PIN,
//			GPIO_MODER_Output,
//			GPIO_OTYPER_PUSH_PULL,
//			GPIO_OSPEEDR_HIGH,
//			GPIO_PUPDR_NO_PULL
//	);
//	gpio_mode(
//			INA3221_SCL_PORT,
//			INA3221_SCL_PIN,
//			GPIO_MODER_Input,
//			GPIO_OTYPER_OPEN_DRAIN,
//			GPIO_OSPEEDR_HIGH,
//			GPIO_PUPDR_PULL_DOWN
//	);

//	while (true) {
//		ina3221_init(0, 4, 4, 68, 30);
//		delay_ms(2000);
//	}

    softi2c_init(INA3221_SCL_PORT, INA3221_SCL_PIN, INA3221_SDA_PORT, INA3221_SDA_PIN);
	while (true) {
//	    bool test = softi2c_probe(INA3221_SCL_PORT, INA3221_SCL_PIN, INA3221_SDA_PORT, INA3221_SDA_PIN, INA3221_ADDRESS);

		softi2c_sigStart(INA3221_SCL_PORT, INA3221_SCL_PIN, INA3221_SDA_PORT, INA3221_SDA_PIN);
		softi2c_send8(INA3221_SCL_PORT, INA3221_SCL_PIN, INA3221_SDA_PORT, INA3221_SDA_PIN, INA3221_ADDRESS << 1 | 1);
		softi2c_readNack(INA3221_SCL_PORT, INA3221_SCL_PIN, INA3221_SDA_PORT, INA3221_SDA_PIN);
		softi2c_sigStop(INA3221_SCL_PORT, INA3221_SCL_PIN, INA3221_SDA_PORT, INA3221_SDA_PIN);

	    delay_ms(2000);
//	    softi2c_writeReg16(INA3221_SCL_PORT, INA3221_SCL_PIN, INA3221_SDA_PORT, INA3221_SDA_PIN, INA3221_ADDRESS, CONFIG_REGISTER, 0x7127);
	}
}

//void testFunction_INA3221() {
////	ina3221_init(0, 4, 4, 68, 30);
//
////	while (true) {
////		ina3221_init(0, 4, 4, 68, 30);
////		nop(5000000);
////	}
//
//	gpio_mode(
//			INA3221_SDA_PORT,
//			INA3221_SDA_PIN,
//			GPIO_MODER_Output,
//			GPIO_OTYPER_PUSH_PULL,
//			GPIO_OSPEEDR_HIGH,
//			GPIO_PUPDR_NO_PULL
//	);
//	gpio_mode(
//			INA3221_SCL_PORT,
//			INA3221_SCL_PIN,
//			GPIO_MODER_Output,
//			GPIO_OTYPER_PUSH_PULL,
//			GPIO_OSPEEDR_HIGH,
//			GPIO_PUPDR_NO_PULL
//	);
//
//	while(true) {
//		gpio_low(INA3221_SDA_PORT, INA3221_SDA_PIN);
//		gpio_high(INA3221_SCL_PORT, INA3221_SCL_PIN);
//		delay_ms(1000);
//
//		gpio_high(INA3221_SDA_PORT, INA3221_SDA_PIN);
//		gpio_low(INA3221_SCL_PORT, INA3221_SCL_PIN);
//		delay_ms(1000);
//	}
//}
