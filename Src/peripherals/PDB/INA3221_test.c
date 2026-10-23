#include "INA3221.h"
#include "GPIO/gpio.h"

void testFunction_INA3221() {
	ina3221_init(0, 4, 4, 68, 30);

	while (true) {
		ina3221_init(0, 4, 4, 68, 30);
		nop(5000000);
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
}
