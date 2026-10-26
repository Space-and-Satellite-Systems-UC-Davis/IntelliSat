#include "INA3221.h"
#include "I2C/i2c.h"
#include "GPIO/gpio.h"
#include "SunSensors/TMP275.h"

void testFunction_INA3221() {
	ina3221_init(0, 4, 4, 68, 30);
    ina3221_getShuntVoltage(INA3221_ADDR_0, CH1_SHUNT_REG,
		INA3221_SDA_PORT, INA3221_SCL_PIN, INA3221_SDA_PIN);
}


// void testFunction_INA3221() {
// 	softi2c_init(INA3221_SCL_PORT, INA3221_SCL_PIN, INA3221_SDA_PORT, INA3221_SDA_PIN);

// 	static int resolution = 0;
// 	static float resolution_conversion = 0;

//     int config = RESOLUTION_12 << 5;

// 	softi2c_lineMode(INA3221_SCL_PORT, INA3221_SCL_PIN, 1);
// 	softi2c_lineMode(INA3221_SDA_PORT, INA3221_SDA_PIN, 1);
//     while(true) {
// //		softi2c_lineMode(INA3221_SCL_PORT, INA3221_SCL_PIN, 1);
// //		softi2c_lineMode(INA3221_SDA_PORT, INA3221_SDA_PIN, 1);
//         softi2c_writeReg16(INA3221_SCL_PORT, INA3221_SCL_PIN, INA3221_SDA_PORT, INA3221_SDA_PIN, TMP0_ADDRESS, 1, config);

//         resolution = 12;
//         resolution_conversion = RESOLUTION_CONVERSION_12;

//         int16_t temp = softi2c_readReg16(INA3221_SCL_PORT, INA3221_SCL_PIN, INA3221_SDA_PORT, INA3221_SDA_PIN, TMP0_ADDRESS, 0);
//         volatile float real_temp = (temp >> (16-resolution))* resolution_conversion;

// //        int16_t id = softi2c_readReg16(INA3221_SCL_PORT, INA3221_SCL_PIN, INA3221_SDA_PORT, INA3221_SDA_PIN, TMP0_ADDRESS, 0);
// //        volatile float real_temp = (temp >> (16-resolution))* resolution_conversion;
// //
// //		softi2c_lineMode(INA3221_SCL_PORT, INA3221_SCL_PIN, 1);
// //		softi2c_lineMode(INA3221_SDA_PORT, INA3221_SDA_PIN, 1);
//         delay_ms(1000);
//     }
// }

//void testFunction_INA3221() {
////	ina3221_init(0, 4, 4, 68, 30);
//
////	gpio_mode(
////			INA3221_SDA_PORT,
////			INA3221_SDA_PIN,
////			GPIO_MODER_Output,
////			GPIO_OTYPER_PUSH_PULL,
////			GPIO_OSPEEDR_HIGH,
////			GPIO_PUPDR_NO_PULL
////	);
////	gpio_mode(
////			INA3221_SCL_PORT,
////			INA3221_SCL_PIN,
////			GPIO_MODER_Input,
////			GPIO_OTYPER_OPEN_DRAIN,
////			GPIO_OSPEEDR_HIGH,
////			GPIO_PUPDR_PULL_DOWN
////	);
//
////	while (true) {
////		ina3221_init(0, 4, 4, 68, 30);
////		delay_ms(2000);
////	}
//
//    softi2c_init(INA3221_SCL_PORT, INA3221_SCL_PIN, INA3221_SDA_PORT, INA3221_SDA_PIN);
//	for (uint8_t i = 0; i <= 0x50; i++) {
////	    bool test = softi2c_probe(INA3221_SCL_PORT, INA3221_SCL_PIN, INA3221_SDA_PORT, INA3221_SDA_PIN, i);
//
//		softi2c_sigStart(INA3221_SCL_PORT, INA3221_SCL_PIN, INA3221_SDA_PORT, INA3221_SDA_PIN);
//		softi2c_send8(INA3221_SCL_PORT, INA3221_SCL_PIN, INA3221_SDA_PORT, INA3221_SDA_PIN, i << 1 | 0);
//		softi2c_readNack(INA3221_SCL_PORT, INA3221_SCL_PIN, INA3221_SDA_PORT, INA3221_SDA_PIN);
//		softi2c_sigStop(INA3221_SCL_PORT, INA3221_SCL_PIN, INA3221_SDA_PORT, INA3221_SDA_PIN);
//
//		// Read manufacturer ID
////		// Write to register pointer
////		softi2c_sigStart(INA3221_SCL_PORT, INA3221_SCL_PIN, INA3221_SDA_PORT, INA3221_SDA_PIN);
////		softi2c_send8(INA3221_SCL_PORT, INA3221_SCL_PIN, INA3221_SDA_PORT, INA3221_SDA_PIN, INA3221_ADDRESS << 1 | 0);
////		softi2c_readNack(INA3221_SCL_PORT, INA3221_SCL_PIN, INA3221_SDA_PORT, INA3221_SDA_PIN);
////		softi2c_send8(INA3221_SCL_PORT, INA3221_SCL_PIN, INA3221_SDA_PORT, INA3221_SDA_PIN, 0xFE);
////		softi2c_readNack(INA3221_SCL_PORT, INA3221_SCL_PIN, INA3221_SDA_PORT, INA3221_SDA_PIN);
////		softi2c_sigStop(INA3221_SCL_PORT, INA3221_SCL_PIN, INA3221_SDA_PORT, INA3221_SDA_PIN);
//
////		int test2 = softi2c_writeReg16(INA3221_SCL_PORT, INA3221_SCL_PIN, INA3221_SDA_PORT, INA3221_SDA_PIN, INA3221_ADDRESS, 0xFE, 0b101010);
//
//
//		// Read back
////		int test3 = softi2c_readReg16(INA3221_SCL_PORT, INA3221_SCL_PIN, INA3221_SDA_PORT, INA3221_SDA_PIN, INA3221_ADDRESS, 0xFE);
//
//	    delay_ms(100);
////	    softi2c_writeReg16(INA3221_SCL_PORT, INA3221_SCL_PIN, INA3221_SDA_PORT, INA3221_SDA_PIN, i, CONFIG_REGISTER, 0x7127);
//	}
//}

//void testFunction_INA3221() {
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
//	for(int i = 0; i < 3; i++) {
//		gpio_low(INA3221_SDA_PORT, INA3221_SDA_PIN);
//		gpio_high(INA3221_SCL_PORT, INA3221_SCL_PIN);
//		delay_ms(1000);
//
//		gpio_high(INA3221_SDA_PORT, INA3221_SDA_PIN);
//		gpio_low(INA3221_SCL_PORT, INA3221_SCL_PIN);
//		delay_ms(1000);
//	}
//	gpio_high(INA3221_SDA_PORT, INA3221_SDA_PIN);
//	gpio_high(INA3221_SCL_PORT, INA3221_SCL_PIN);
//}
