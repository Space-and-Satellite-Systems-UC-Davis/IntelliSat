/*
 * DMA_test.c
 *
 *  Created on: Oct 30, 2025
 */

#include "SunSensors/sun_sensors.h"
#include "ADC/adc.h"
#include "DMA/DMA.h"
#include "IMU/ASM330LHH.h"
#include "Buttons/buttons.h"
#include <print_scan.h>

void testFunction_DMA_ADC() {
    printMsg("START\r\n");

    uint16_t test_buffer[1] = {42};
	printMsg("\r\nARRAY (We Expect 42): %u\r\n", test_buffer[0]);

    DMAConfig config;
    config.selection = SELECT_ADC2;
    config.peripheral_addr = (uint32_t) &(ADC2->DR);
    config.memory_addr = (uint32_t)&test_buffer;
    config.length = 1;
    config.pdata_size = 1;
    config.mdata_size = 1;
    config.circular = false;
    config.peripheral_to_memory = true;
    config.peripheral_increment = false;
    config.memory_increment = false;
    config.transfer_interrupt = false;

    dma_configure_channel(config);
    adc_continuous_dma(ADC2, 2);
    dma_enable_channel(SELECT_ADC2);
    while ((ADC2->ISR & ADC_ISR_EOC) == 0) nop(1);
	printMsg("\r\nARRAY (We Expect not 42): %u\r\n", test_buffer[0]);
    dma_disable_channel(SELECT_ADC2);



    test_buffer[0] = 24;
	printMsg("\r\nARRAY (We Expect 24): %u\r\n", test_buffer[0]);

    config.selection = SELECT_ADC3;
    config.peripheral_addr = (uint32_t) &(ADC3->DR);
    config.memory_addr = (uint32_t)&test_buffer;
    config.length = 1;
    config.pdata_size = 1;
    config.mdata_size = 1;
    config.circular = true;
    config.peripheral_to_memory = true;
    config.peripheral_increment = false;
    config.memory_increment = false;
    config.transfer_interrupt = false;

    dma_configure_channel(config);
    adc_continuous_dma(ADC3, 6);
    dma_enable_channel(SELECT_ADC3);
    while ((ADC3->ISR & ADC_ISR_EOC) == 0) nop(1);
	printMsg("\r\nARRAY (We Expect not 24): %u\r\n", test_buffer[0]);
//    dma_disable_channel(SELECT_ADC3);
//
//
//
//    test_buffer[0] = 12;
//	printMsg("\r\nARRAY (We Expect 12): %u\r\n", test_buffer[0]);
//
//    config.selection = SELECT_ADC1;
//    config.peripheral_addr = (uint32_t) &(ADC1->DR);
//    config.memory_addr = (uint32_t)&test_buffer;
//    config.length = 1;
//    config.pdata_size = 2;
//    config.mdata_size = 2;
//    config.circular = true;
//    config.peripheral_to_memory = true;
//    config.peripheral_increment = false;
//    config.memory_increment = false;
//    config.transfer_interrupt = false;
//
//    configure_channel(config);
//    adc_continuous_dma(ADC1, 6);
//    dma_enable_channel(SELECT_ADC1);
//    while ((ADC1->ISR & ADC_ISR_EOC) == 0) nop(1);
//	printMsg("\r\nARRAY (We Expect not 12): %u\r\n", test_buffer[0]);
//    dma_disable_channel(SELECT_ADC1);

	// Connect pin 6 to ground on ADC3
	// If the array and register both go to zero, success

	while (1) {
		printMsg("\r\nREGISTER: %u\r\n", ADC3->DR);
		printMsg("ARRAY (Should change continuously): %u\r\n", test_buffer[0]);

		nop(900000);
		break;
	}
}
